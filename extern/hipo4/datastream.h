//******************************************************************************
//*       ██╗  ██╗██╗██████╗  ██████╗     ██╗  ██╗    ██████╗                  *
//*       ██║  ██║██║██╔══██╗██╔═══██╗    ██║  ██║   ██╔═████╗                 *
//*       ███████║██║██████╔╝██║   ██║    ███████║   ██║██╔██║                 *
//*       ██╔══██║██║██╔═══╝ ██║   ██║    ╚════██║   ████╔╝██║                 *
//*       ██║  ██║██║██║     ╚██████╔╝         ██║██╗╚██████╔╝                 *
//*       ╚═╝  ╚═╝╚═╝╚═╝      ╚═════╝          ╚═╝╚═╝ ╚═════╝                  *
//************************ Jefferson National Lab (2017) ***********************
//
// Polymorphic byte-stream abstraction backing hipo::reader / hipo::FileHandle.
//
// Three concrete backends:
//   * datastreamLocalFile : std::ifstream-backed local read.
//   * datastreamXrootd    : XRootD (`root://`) — gated on __XROOTD__.
//   * datastreamROOT      : TFile-backed (`root://`, `http(s)://`, `davix://`,
//                            and any other protocol ROOT supports) — gated on
//                            __ROOT__.
//
// Use make_datastream(url) to construct the right backend from a URL/path.
// All implementations expose the same minimal seek+read interface so callers
// don't need to care whether the bytes live on local disk or across a network.
//

#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef __XROOTD__
#include <XrdClient/XrdClient.hh>
#include "XrdClient/XrdClientEnv.hh"
#include "XrdSys/XrdSysHeaders.hh"
#endif

#ifdef __ROOT__
class TFile;  // forward declare; full include lives in datastream.cpp
#endif

namespace hipo {

///
/// @brief Abstract base for a byte-stream reader.
///
/// All concrete datastreams support absolute-position seeking via `position(pos)`
/// and synchronous `read()` into a caller-owned buffer. Implementations are NOT
/// required to be thread-safe — wrap with a mutex if multiple threads share one.
///
class datastream {
public:
    datastream() = default;
    datastream(const datastream&) = delete;
    datastream& operator=(const datastream&) = delete;
    virtual ~datastream() = default;

    /// Open the backing file/URL. Throws hipo::file_error on failure.
    virtual void open(const char* filename) = 0;

    /// Total size in bytes (or -1 if unknown).
    virtual long size() = 0;

    /// Current read offset.
    virtual long position() = 0;

    /// Seek to absolute offset `pos`. Returns the new position.
    virtual long position(long pos) = 0;

    /// Synchronous read of up to `size` bytes into `buffer`. Returns bytes read.
    virtual int read(char* buffer, int size) = 0;

    /// Vectored read: convenience helper. Default impl issues one seek+read per
    /// range; backends with native scatter/gather (e.g. TFile::ReadBuffers) can
    /// override to coalesce them into a single network round-trip.
    /// Each request is {absolute_offset, byte_count, output_buffer}.
    struct read_range {
        long offset;
        int  bytes;
        char* buffer;
    };
    virtual void read_many(const std::vector<read_range>& requests);
};

///
/// @brief Local-file backend using std::ifstream.
///
class datastreamLocalFile : public datastream {
public:
    datastreamLocalFile() = default;

    ~datastreamLocalFile() override {
        if (inputStream.is_open()) inputStream.close();
    }

    void open(const char* filename) override;
    long size() override;
    long position() override {
        // After a partial read or EOF, tellg may return -1 unless we clear.
        if (inputStream.fail() && !inputStream.bad()) inputStream.clear();
        return static_cast<long>(inputStream.tellg());
    }
    long position(long pos) override {
        // seekg fails silently on a streamthat already hit EOF/failbit; reset
        // before seeking so the caller always lands at `pos`.
        if (inputStream.fail() && !inputStream.bad()) inputStream.clear();
        inputStream.seekg(pos, std::ios_base::beg);
        return pos;
    }
    int  read(char* s, int n) override {
        inputStream.read(s, n);
        int actual = static_cast<int>(inputStream.gcount());
        // A speculative read may legitimately overshoot EOF (e.g., when
        // grabbing the trailer record near the end of the file). Clear the
        // failbit so subsequent seek+read calls work.
        if (inputStream.eof() || (inputStream.fail() && !inputStream.bad())) {
            inputStream.clear();
        }
        return actual;
    }

private:
    std::ifstream inputStream;
};

///
/// @brief XRootD backend. Skeleton; gated on __XROOTD__.
///
class datastreamXrootd : public datastream {
public:
    datastreamXrootd();
    ~datastreamXrootd() override;

    void open(const char* filename) override;
    long size() override;
    long position() override { return streamPosition; }
    long position(long pos) override { streamPosition = pos; return streamPosition; }
    int  read(char* s, int n) override;

private:
#ifdef __XROOTD__
    kXR_unt16 open_mode = (kXR_ur);
    kXR_unt16 open_opts = (1);
    XrdClient* cli = nullptr;
#endif
    long streamPosition = 0;
};

///
/// @brief ROOT-TFile backend. Handles root://, http(s)://, davix://, etc. via
/// ROOT's TFile::Open. Vectored reads use TFile::ReadBuffers when available.
///
/// Only compiled in when __ROOT__ is defined.
///
class datastreamROOT : public datastream {
public:
    datastreamROOT();
    ~datastreamROOT() override;

    void open(const char* filename) override;
    long size() override;
    long position() override { return streamPosition; }
    long position(long pos) override { streamPosition = pos; return streamPosition; }
    int  read(char* s, int n) override;
    void read_many(const std::vector<read_range>& requests) override;

private:
#ifdef __ROOT__
    TFile* file = nullptr;
#endif
    long streamPosition = 0;
    long streamSize     = -1;
};

///
/// @brief Factory: choose a backend based on the URL scheme.
///
/// Scheme rules:
///   no-scheme / file://         -> datastreamLocalFile
///   root://                     -> datastreamROOT if __ROOT__, else datastreamXrootd
///   http://, https://, davix:// -> datastreamROOT (throws if not built with ROOT)
///
/// Returns nullptr only if `url` is empty.
///
std::unique_ptr<datastream> make_datastream(std::string_view url);

///
/// @brief Classify a URL/path as remote (true) or local (false).
///
bool is_remote_url(std::string_view url) noexcept;

} // namespace hipo
#endif /* DATASTREAM_H */
