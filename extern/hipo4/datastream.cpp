//******************************************************************************
//*       ██╗  ██╗██╗██████╗  ██████╗     ██╗  ██╗    ██████╗                  *
//*       ██║  ██║██║██╔══██╗██╔═══██╗    ██║  ██║   ██╔═████╗                 *
//*       ███████║██║██████╔╝██║   ██║    ███████║   ██║██╔██║                 *
//*       ██╔══██║██║██╔═══╝ ██║   ██║    ╚════██║   ████╔╝██║                 *
//*       ██║  ██║██║██║     ╚██████╔╝         ██║██╗╚██████╔╝                 *
//*       ╚═╝  ╚═╝╚═╝╚═╝      ╚═════╝          ╚═╝╚═╝ ╚═════╝                  *
//************************ Jefferson National Lab (2017) ***********************

#include "datastream.h"

#include "hipoexceptions.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#ifdef __ROOT__
#include "TFile.h"
#include "TROOT.h"
#endif

namespace hipo {

// -------------------------------------------------------------------------
// datastream base class: default vectored read = sequential reads.
// -------------------------------------------------------------------------
void datastream::read_many(const std::vector<read_range>& requests) {
    for (const auto& r : requests) {
        position(r.offset);
        read(r.buffer, r.bytes);
    }
}

// -------------------------------------------------------------------------
// datastreamLocalFile
// -------------------------------------------------------------------------
void datastreamLocalFile::open(const char* filename) {
    inputStream.open(filename, std::ios::binary);
    if (!inputStream.is_open()) {
        throw file_error(std::string("datastreamLocalFile: cannot open ") + filename);
    }
}

long datastreamLocalFile::size() {
    auto pos = inputStream.tellg();
    inputStream.seekg(0, std::ios_base::end);
    auto lastpos = inputStream.tellg();
    inputStream.seekg(pos, std::ios_base::beg);
    return static_cast<long>(lastpos);
}

// -------------------------------------------------------------------------
// datastreamXrootd
// -------------------------------------------------------------------------
datastreamXrootd::datastreamXrootd() = default;

datastreamXrootd::~datastreamXrootd() {
#ifdef __XROOTD__
    if (cli != nullptr) delete cli;
#endif
}

void datastreamXrootd::open(const char* filename) {
#ifdef __XROOTD__
    cli = new XrdClient(filename);
    if (!cli->Open(open_mode, open_opts)) {
        throw file_error(std::string("datastreamXrootd: cannot open ") + filename);
    }
#else
    (void)filename;
    throw file_error("datastreamXrootd: built without __XROOTD__ support");
#endif
}

long datastreamXrootd::size() {
#ifdef __XROOTD__
    XrdClientStatInfo stats;
    cli->Stat(&stats);
    return stats.size;
#else
    return 0;
#endif
}

int datastreamXrootd::read(char* s, int n) {
#ifdef __XROOTD__
    cli->Read(s, streamPosition, n);
    streamPosition += n;
    return n;
#else
    (void)s; (void)n;
    return 0;
#endif
}

// -------------------------------------------------------------------------
// datastreamROOT
// -------------------------------------------------------------------------
datastreamROOT::datastreamROOT() = default;

datastreamROOT::~datastreamROOT() {
#ifdef __ROOT__
    if (file != nullptr) {
        file->Close();
        delete file;
        file = nullptr;
    }
#endif
}

void datastreamROOT::open(const char* filename) {
#ifdef __ROOT__
    file = TFile::Open(filename, "READ");
    if (file == nullptr || file->IsZombie()) {
        throw file_error(std::string("datastreamROOT: cannot open ") + filename);
    }
    streamSize = static_cast<long>(file->GetSize());
    streamPosition = 0;
#else
    (void)filename;
    throw file_error("datastreamROOT: built without __ROOT__ support");
#endif
}

long datastreamROOT::size() {
#ifdef __ROOT__
    return streamSize;
#else
    return 0;
#endif
}

int datastreamROOT::read(char* s, int n) {
#ifdef __ROOT__
    if (file == nullptr) return 0;
    if (file->ReadBuffer(s, streamPosition, n)) {
        // TFile::ReadBuffer returns true on FAILURE (kTRUE/1).
        return 0;
    }
    streamPosition += n;
    return n;
#else
    (void)s; (void)n;
    return 0;
#endif
}

void datastreamROOT::read_many(const std::vector<read_range>& requests) {
#ifdef __ROOT__
    if (requests.empty() || file == nullptr) return;

    // Use TFile::ReadBuffers (vectored I/O) to coalesce into one network RTT.
    std::vector<Long64_t> positions;
    std::vector<Int_t>    lengths;
    positions.reserve(requests.size());
    lengths.reserve(requests.size());

    // Compute total output size and pack into a single contiguous staging buffer.
    long total = 0;
    for (const auto& r : requests) {
        positions.push_back(static_cast<Long64_t>(r.offset));
        lengths.push_back(static_cast<Int_t>(r.bytes));
        total += r.bytes;
    }
    std::vector<char> staging(static_cast<std::size_t>(total));
    if (file->ReadBuffers(staging.data(),
                          positions.data(),
                          lengths.data(),
                          static_cast<Int_t>(requests.size()))) {
        // failure: fall back to sequential reads
        datastream::read_many(requests);
        return;
    }
    long off = 0;
    for (const auto& r : requests) {
        std::memcpy(r.buffer, staging.data() + off, r.bytes);
        off += r.bytes;
    }
#else
    datastream::read_many(requests);
#endif
}

// -------------------------------------------------------------------------
// URL scheme classification + factory
// -------------------------------------------------------------------------
namespace {

bool starts_with(std::string_view s, std::string_view prefix) noexcept {
    return s.size() >= prefix.size() &&
           std::equal(prefix.begin(), prefix.end(), s.begin(),
                      [](char a, char b) {
                          return std::tolower(static_cast<unsigned char>(a)) ==
                                 std::tolower(static_cast<unsigned char>(b));
                      });
}

} // namespace

bool is_remote_url(std::string_view url) noexcept {
    if (url.empty()) return false;
    return starts_with(url, "root://")  ||
           starts_with(url, "http://")  ||
           starts_with(url, "https://") ||
           starts_with(url, "davix://") ||
           starts_with(url, "dav://")   ||
           starts_with(url, "s3://")    ||
           starts_with(url, "gs://");
}

std::unique_ptr<datastream> make_datastream(std::string_view url) {
    if (url.empty()) return nullptr;

    // root:// — prefer ROOT TFile if available (more features), else XRootD.
    if (starts_with(url, "root://")) {
#ifdef __ROOT__
        return std::make_unique<datastreamROOT>();
#elif defined(__XROOTD__)
        return std::make_unique<datastreamXrootd>();
#else
        throw file_error("hipo::make_datastream: 'root://' URL requires "
                         "either __ROOT__ or __XROOTD__ build support");
#endif
    }
    // Anything else web-like → ROOT TFile.
    if (is_remote_url(url)) {
#ifdef __ROOT__
        return std::make_unique<datastreamROOT>();
#else
        throw file_error("hipo::make_datastream: remote URL requires "
                         "__ROOT__ build support");
#endif
    }
    // file:// or path → local.
    return std::make_unique<datastreamLocalFile>();
}

} // namespace hipo
