//******************************************************************************
// clas12.h - AUTO-GENERATED typed accessors for CLAS12 reconstructed event banks
//
// Generated from etc/bankdefs/event.json by scripts/generate_clas12.py
// DO NOT EDIT - regenerate with: python3 scripts/generate_clas12.py
//******************************************************************************

#pragma once

#include "bank.h"
#include "dictionary.h"

namespace clas12 {

namespace rec {

    /// Event Header Bank
    class Event {
    public:
        explicit Event(hipo::bank& b) : b_(b),
            category_(b.getSchema().getEntryOrder("category")),
            topology_(b.getSchema().getEntryOrder("topology")),
            beamCharge_(b.getSchema().getEntryOrder("beamCharge")),
            liveTime_(b.getSchema().getEntryOrder("liveTime")),
            startTime_(b.getSchema().getEntryOrder("startTime")),
            RFTime_(b.getSchema().getEntryOrder("RFTime")),
            helicity_(b.getSchema().getEntryOrder("helicity")),
            helicityRaw_(b.getSchema().getEntryOrder("helicityRaw")),
            procTime_(b.getSchema().getEntryOrder("procTime"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// Undefined
        long category(int row) const noexcept { return b_.getLong(category_, row); }
        /// Undefined
        long topology(int row) const noexcept { return b_.getLong(topology_, row); }
        /// Beam charge, gated (nano-Coulomb)
        float beamCharge(int row) const noexcept { return b_.getFloat(beamCharge_, row); }
        /// Lifetime
        double liveTime(int row) const noexcept { return b_.getDouble(liveTime_, row); }
        /// Event Start Time (ns)
        float startTime(int row) const noexcept { return b_.getFloat(startTime_, row); }
        /// RF Time (ns)
        float RFTime(int row) const noexcept { return b_.getFloat(RFTime_, row); }
        /// Helicity of Event (+/-1, else undefined), with HWP-correction
        int helicity(int row) const noexcept { return b_.getInt(helicity_, row); }
        /// Helicity of Event (+/-1, else undefined)
        int helicityRaw(int row) const noexcept { return b_.getInt(helicityRaw_, row); }
        /// Event Processing Time (UNIX Time = seconds)
        float procTime(int row) const noexcept { return b_.getFloat(procTime_, row); }

        void set_category(int row, int64_t val) { b_.putLong(category_, row, val); }
        void set_topology(int row, int64_t val) { b_.putLong(topology_, row, val); }
        void set_beamCharge(int row, float val) { b_.putFloat(beamCharge_, row, val); }
        void set_liveTime(int row, double val) { b_.putDouble(liveTime_, row, val); }
        void set_startTime(int row, float val) { b_.putFloat(startTime_, row, val); }
        void set_RFTime(int row, float val) { b_.putFloat(RFTime_, row, val); }
        void set_helicity(int row, int8_t val) { b_.putByte(helicity_, row, val); }
        void set_helicityRaw(int row, int8_t val) { b_.putByte(helicityRaw_, row, val); }
        void set_procTime(int row, float val) { b_.putFloat(procTime_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::Event", 300, 30);
            s.parse("category/L,topology/L,beamCharge/F,liveTime/D,startTime/F,RFTime/F,helicity/B,helicityRaw/B,procTime/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int category_;
        const int topology_;
        const int beamCharge_;
        const int liveTime_;
        const int startTime_;
        const int RFTime_;
        const int helicity_;
        const int helicityRaw_;
        const int procTime_;
    };

    /// Reconstructed Particle Information
    class Particle {
    public:
        explicit Particle(hipo::bank& b) : b_(b),
            pid_(b.getSchema().getEntryOrder("pid")),
            px_(b.getSchema().getEntryOrder("px")),
            py_(b.getSchema().getEntryOrder("py")),
            pz_(b.getSchema().getEntryOrder("pz")),
            vx_(b.getSchema().getEntryOrder("vx")),
            vy_(b.getSchema().getEntryOrder("vy")),
            vz_(b.getSchema().getEntryOrder("vz")),
            vt_(b.getSchema().getEntryOrder("vt")),
            charge_(b.getSchema().getEntryOrder("charge")),
            beta_(b.getSchema().getEntryOrder("beta")),
            chi2pid_(b.getSchema().getEntryOrder("chi2pid")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// particle id in LUND conventions
        int pid(int row) const noexcept { return b_.getInt(pid_, row); }
        /// x component of the momentum (GeV)
        float px(int row) const noexcept { return b_.getFloat(px_, row); }
        /// y component of the momentum (GeV)
        float py(int row) const noexcept { return b_.getFloat(py_, row); }
        /// z component of the momentum (GeV)
        float pz(int row) const noexcept { return b_.getFloat(pz_, row); }
        /// x component of the vertex (cm)
        float vx(int row) const noexcept { return b_.getFloat(vx_, row); }
        /// y component of the vertex (cm)
        float vy(int row) const noexcept { return b_.getFloat(vy_, row); }
        /// z component of the vertex (cm)
        float vz(int row) const noexcept { return b_.getFloat(vz_, row); }
        /// RF and z corrected vertex time (ns)
        float vt(int row) const noexcept { return b_.getFloat(vt_, row); }
        /// particle charge
        int charge(int row) const noexcept { return b_.getInt(charge_, row); }
        /// particle beta measured by TOF
        float beta(int row) const noexcept { return b_.getFloat(beta_, row); }
        /// Chi2 of assigned PID
        float chi2pid(int row) const noexcept { return b_.getFloat(chi2pid_, row); }
        /// particle status (represents detector collection it passed)
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_pid(int row, int32_t val) { b_.putInt(pid_, row, val); }
        void set_px(int row, float val) { b_.putFloat(px_, row, val); }
        void set_py(int row, float val) { b_.putFloat(py_, row, val); }
        void set_pz(int row, float val) { b_.putFloat(pz_, row, val); }
        void set_vx(int row, float val) { b_.putFloat(vx_, row, val); }
        void set_vy(int row, float val) { b_.putFloat(vy_, row, val); }
        void set_vz(int row, float val) { b_.putFloat(vz_, row, val); }
        void set_vt(int row, float val) { b_.putFloat(vt_, row, val); }
        void set_charge(int row, int8_t val) { b_.putByte(charge_, row, val); }
        void set_beta(int row, float val) { b_.putFloat(beta_, row, val); }
        void set_chi2pid(int row, float val) { b_.putFloat(chi2pid_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::Particle", 300, 31);
            s.parse("pid/I,px/F,py/F,pz/F,vx/F,vy/F,vz/F,vt/F,charge/B,beta/F,chi2pid/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int pid_;
        const int px_;
        const int py_;
        const int pz_;
        const int vx_;
        const int vy_;
        const int vz_;
        const int vt_;
        const int charge_;
        const int beta_;
        const int chi2pid_;
        const int status_;
    };

    /// Calorimeter Responses for Particles bank
    class Calorimeter {
    public:
        explicit Calorimeter(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            energy_(b.getSchema().getEntryOrder("energy")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            hx_(b.getSchema().getEntryOrder("hx")),
            hy_(b.getSchema().getEntryOrder("hy")),
            hz_(b.getSchema().getEntryOrder("hz")),
            lu_(b.getSchema().getEntryOrder("lu")),
            lv_(b.getSchema().getEntryOrder("lv")),
            lw_(b.getSchema().getEntryOrder("lw")),
            du_(b.getSchema().getEntryOrder("du")),
            dv_(b.getSchema().getEntryOrder("dv")),
            dw_(b.getSchema().getEntryOrder("dw")),
            m2u_(b.getSchema().getEntryOrder("m2u")),
            m2v_(b.getSchema().getEntryOrder("m2v")),
            m2w_(b.getSchema().getEntryOrder("m2w")),
            m3u_(b.getSchema().getEntryOrder("m3u")),
            m3v_(b.getSchema().getEntryOrder("m3v")),
            m3w_(b.getSchema().getEntryOrder("m3w")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the hit in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the Detector hit
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// Layer ID, as defined in org.jlab.detector.base.DetectorLayer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Energy associated with the hit (GeV)
        float energy(int row) const noexcept { return b_.getFloat(energy_, row); }
        /// Time associated with the hit (ns)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the hit position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of hit-track matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the hit (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the hit (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the hit (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// X coordinate of the matched hit (cm)
        float hx(int row) const noexcept { return b_.getFloat(hx_, row); }
        /// Y coordinate of the mathced hit (cm)
        float hy(int row) const noexcept { return b_.getFloat(hy_, row); }
        /// Z coordinate of the matched hit (cm)
        float hz(int row) const noexcept { return b_.getFloat(hz_, row); }
        /// distance on U-side
        float lu(int row) const noexcept { return b_.getFloat(lu_, row); }
        /// distance on V-side
        float lv(int row) const noexcept { return b_.getFloat(lv_, row); }
        /// distance on W-side
        float lw(int row) const noexcept { return b_.getFloat(lw_, row); }
        /// shower width on U-side
        float du(int row) const noexcept { return b_.getFloat(du_, row); }
        /// shower width on V-side
        float dv(int row) const noexcept { return b_.getFloat(dv_, row); }
        /// shower width on w-side
        float dw(int row) const noexcept { return b_.getFloat(dw_, row); }
        /// 2nd moment of the shower on U-side
        float m2u(int row) const noexcept { return b_.getFloat(m2u_, row); }
        /// 2nd moment of the shower on V-side
        float m2v(int row) const noexcept { return b_.getFloat(m2v_, row); }
        /// 2nd moment of the shower on W-side
        float m2w(int row) const noexcept { return b_.getFloat(m2w_, row); }
        /// 3rd moment of the shower on U-side
        float m3u(int row) const noexcept { return b_.getFloat(m3u_, row); }
        /// 3rd moment of the shower on V-side
        float m3v(int row) const noexcept { return b_.getFloat(m3v_, row); }
        /// 3rd moment of the shower on W-side
        float m3w(int row) const noexcept { return b_.getFloat(m3w_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_energy(int row, float val) { b_.putFloat(energy_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_hx(int row, float val) { b_.putFloat(hx_, row, val); }
        void set_hy(int row, float val) { b_.putFloat(hy_, row, val); }
        void set_hz(int row, float val) { b_.putFloat(hz_, row, val); }
        void set_lu(int row, float val) { b_.putFloat(lu_, row, val); }
        void set_lv(int row, float val) { b_.putFloat(lv_, row, val); }
        void set_lw(int row, float val) { b_.putFloat(lw_, row, val); }
        void set_du(int row, float val) { b_.putFloat(du_, row, val); }
        void set_dv(int row, float val) { b_.putFloat(dv_, row, val); }
        void set_dw(int row, float val) { b_.putFloat(dw_, row, val); }
        void set_m2u(int row, float val) { b_.putFloat(m2u_, row, val); }
        void set_m2v(int row, float val) { b_.putFloat(m2v_, row, val); }
        void set_m2w(int row, float val) { b_.putFloat(m2w_, row, val); }
        void set_m3u(int row, float val) { b_.putFloat(m3u_, row, val); }
        void set_m3v(int row, float val) { b_.putFloat(m3v_, row, val); }
        void set_m3w(int row, float val) { b_.putFloat(m3w_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::Calorimeter", 300, 32);
            s.parse("index/S,pindex/S,detector/B,sector/B,layer/B,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,hx/F,hy/F,hz/F,lu/F,lv/F,lw/F,du/F,dv/F,dw/F,m2u/F,m2v/F,m2w/F,m3u/F,m3v/F,m3w/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int layer_;
        const int energy_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int hx_;
        const int hy_;
        const int hz_;
        const int lu_;
        const int lv_;
        const int lw_;
        const int du_;
        const int dv_;
        const int dw_;
        const int m2u_;
        const int m2v_;
        const int m2w_;
        const int m3u_;
        const int m3v_;
        const int m3w_;
        const int status_;
    };

    /// Cherenkov Responses for Particles bank
    class Cherenkov {
    public:
        explicit Cherenkov(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            nphe_(b.getSchema().getEntryOrder("nphe")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            dtheta_(b.getSchema().getEntryOrder("dtheta")),
            dphi_(b.getSchema().getEntryOrder("dphi")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the hit in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the Detector hit
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// Number of photoelectrons from Cherenkov radiation
        float nphe(int row) const noexcept { return b_.getFloat(nphe_, row); }
        /// Time associated with the hit (ns)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the hit position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of hit-track matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the hit (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the hit (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the hit (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// Expected Theta Resolution (deg)
        float dtheta(int row) const noexcept { return b_.getFloat(dtheta_, row); }
        /// Expected Phi Resolution (deg)
        float dphi(int row) const noexcept { return b_.getFloat(dphi_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_nphe(int row, float val) { b_.putFloat(nphe_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_dtheta(int row, float val) { b_.putFloat(dtheta_, row, val); }
        void set_dphi(int row, float val) { b_.putFloat(dphi_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::Cherenkov", 300, 33);
            s.parse("index/S,pindex/S,detector/B,sector/B,nphe/F,time/F,path/F,chi2/F,x/F,y/F,z/F,dtheta/F,dphi/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int nphe_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int dtheta_;
        const int dphi_;
        const int status_;
    };

    /// Forward Tagger information for Particles bank
    class ForwardTagger {
    public:
        explicit ForwardTagger(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            energy_(b.getSchema().getEntryOrder("energy")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            dx_(b.getSchema().getEntryOrder("dx")),
            dy_(b.getSchema().getEntryOrder("dy")),
            radius_(b.getSchema().getEntryOrder("radius")),
            size_(b.getSchema().getEntryOrder("size")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the cluster in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// layer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Energy associated with the cluster (GeV)
        float energy(int row) const noexcept { return b_.getFloat(energy_, row); }
        /// Time associated with the cluster (ns)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the cluster position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of cluster-particle matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the cluster
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the cluster
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the cluster
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// Cluster width in x
        float dx(int row) const noexcept { return b_.getFloat(dx_, row); }
        /// Cluster width in y
        float dy(int row) const noexcept { return b_.getFloat(dy_, row); }
        /// Cluster radius
        float radius(int row) const noexcept { return b_.getFloat(radius_, row); }
        /// size of the cluster
        int size(int row) const noexcept { return b_.getInt(size_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_energy(int row, float val) { b_.putFloat(energy_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_dx(int row, float val) { b_.putFloat(dx_, row, val); }
        void set_dy(int row, float val) { b_.putFloat(dy_, row, val); }
        void set_radius(int row, float val) { b_.putFloat(radius_, row, val); }
        void set_size(int row, int16_t val) { b_.putShort(size_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::ForwardTagger", 300, 34);
            s.parse("index/S,pindex/S,detector/B,layer/B,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,dx/F,dy/F,radius/F,size/S,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int layer_;
        const int energy_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int dx_;
        const int dy_;
        const int radius_;
        const int size_;
        const int status_;
    };

    /// Scintillator Responses for Particles bank
    class Scintillator {
    public:
        explicit Scintillator(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            component_(b.getSchema().getEntryOrder("component")),
            energy_(b.getSchema().getEntryOrder("energy")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            hx_(b.getSchema().getEntryOrder("hx")),
            hy_(b.getSchema().getEntryOrder("hy")),
            hz_(b.getSchema().getEntryOrder("hz")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the hit in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the Detector hit
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// Layer ID, as defined in org.jlab.detector.base.DetectorLayer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Component of the Detector hit
        int component(int row) const noexcept { return b_.getInt(component_, row); }
        /// Energy associated with the hit (MeV)
        float energy(int row) const noexcept { return b_.getFloat(energy_, row); }
        /// Time associated with the hit (ns)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the hit position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of hit-track matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the hit (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the hit (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the hit (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// X coordinate of the matched hit (cm)
        float hx(int row) const noexcept { return b_.getFloat(hx_, row); }
        /// Y coordinate of the mathced hit (cm)
        float hy(int row) const noexcept { return b_.getFloat(hy_, row); }
        /// Z coordinate of the matched hit (cm)
        float hz(int row) const noexcept { return b_.getFloat(hz_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_component(int row, int16_t val) { b_.putShort(component_, row, val); }
        void set_energy(int row, float val) { b_.putFloat(energy_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_hx(int row, float val) { b_.putFloat(hx_, row, val); }
        void set_hy(int row, float val) { b_.putFloat(hy_, row, val); }
        void set_hz(int row, float val) { b_.putFloat(hz_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::Scintillator", 300, 35);
            s.parse("index/S,pindex/S,detector/B,sector/B,layer/B,component/S,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,hx/F,hy/F,hz/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int layer_;
        const int component_;
        const int energy_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int hx_;
        const int hy_;
        const int hz_;
        const int status_;
    };

    /// Scintillator Cluster Parameters
    class ScintExtras {
    public:
        explicit ScintExtras(hipo::bank& b) : b_(b),
            dedx_(b.getSchema().getEntryOrder("dedx")),
            size_(b.getSchema().getEntryOrder("size")),
            layermult_(b.getSchema().getEntryOrder("layermult"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// Energy deposition per distance travelled (MeV/cm)
        float dedx(int row) const noexcept { return b_.getFloat(dedx_, row); }
        /// cluster size
        int size(int row) const noexcept { return b_.getInt(size_, row); }
        /// cluster layer multiplicity
        int layermult(int row) const noexcept { return b_.getInt(layermult_, row); }

        void set_dedx(int row, float val) { b_.putFloat(dedx_, row, val); }
        void set_size(int row, int16_t val) { b_.putShort(size_, row, val); }
        void set_layermult(int row, int8_t val) { b_.putByte(layermult_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::ScintExtras", 300, 43);
            s.parse("dedx/F,size/S,layermult/B");
            return s;
        }

    private:
        hipo::bank& b_;
        const int dedx_;
        const int size_;
        const int layermult_;
    };

    /// Calorimeter Cluster Parameters
    class CaloExtras {
    public:
        explicit CaloExtras(hipo::bank& b) : b_(b),
            dbstU_(b.getSchema().getEntryOrder("dbstU")),
            dbstV_(b.getSchema().getEntryOrder("dbstV")),
            dbstW_(b.getSchema().getEntryOrder("dbstW")),
            rawEU_(b.getSchema().getEntryOrder("rawEU")),
            rawEV_(b.getSchema().getEntryOrder("rawEV")),
            rawEW_(b.getSchema().getEntryOrder("rawEW")),
            recEU_(b.getSchema().getEntryOrder("recEU")),
            recEV_(b.getSchema().getEntryOrder("recEV")),
            recEW_(b.getSchema().getEntryOrder("recEW")),
            recDTU_(b.getSchema().getEntryOrder("recDTU")),
            recDTV_(b.getSchema().getEntryOrder("recDTV")),
            recDTW_(b.getSchema().getEntryOrder("recDTW")),
            recFTU_(b.getSchema().getEntryOrder("recFTU")),
            recFTV_(b.getSchema().getEntryOrder("recFTV")),
            recFTW_(b.getSchema().getEntryOrder("recFTW")),
            size_(b.getSchema().getEntryOrder("size"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// DB status + U strip*10
        int dbstU(int row) const noexcept { return b_.getInt(dbstU_, row); }
        /// DB status + V strip*10
        int dbstV(int row) const noexcept { return b_.getInt(dbstV_, row); }
        /// DB status + W strip*10
        int dbstW(int row) const noexcept { return b_.getInt(dbstW_, row); }
        /// raw U peak energy
        float rawEU(int row) const noexcept { return b_.getFloat(rawEU_, row); }
        /// raw V peak energy
        float rawEV(int row) const noexcept { return b_.getFloat(rawEV_, row); }
        /// raw W peak energy
        float rawEW(int row) const noexcept { return b_.getFloat(rawEW_, row); }
        /// recon U peak energy (GeV)
        float recEU(int row) const noexcept { return b_.getFloat(recEU_, row); }
        /// recon V peak energy (GeV)
        float recEV(int row) const noexcept { return b_.getFloat(recEV_, row); }
        /// recon W peak energy (GeV)
        float recEW(int row) const noexcept { return b_.getFloat(recEW_, row); }
        /// recon U peak TDC time (ns)
        float recDTU(int row) const noexcept { return b_.getFloat(recDTU_, row); }
        /// recon V peak TDC time (ns)
        float recDTV(int row) const noexcept { return b_.getFloat(recDTV_, row); }
        /// recon W peak TDC time (ns)
        float recDTW(int row) const noexcept { return b_.getFloat(recDTW_, row); }
        /// recon U peak FADC time (ns)
        float recFTU(int row) const noexcept { return b_.getFloat(recFTU_, row); }
        /// recon V peak FADC time (ns)
        float recFTV(int row) const noexcept { return b_.getFloat(recFTV_, row); }
        /// recon W peak FADC time (ns)
        float recFTW(int row) const noexcept { return b_.getFloat(recFTW_, row); }
        /// cluster size
        float size(int row) const noexcept { return b_.getFloat(size_, row); }

        void set_dbstU(int row, int16_t val) { b_.putShort(dbstU_, row, val); }
        void set_dbstV(int row, int16_t val) { b_.putShort(dbstV_, row, val); }
        void set_dbstW(int row, int16_t val) { b_.putShort(dbstW_, row, val); }
        void set_rawEU(int row, float val) { b_.putFloat(rawEU_, row, val); }
        void set_rawEV(int row, float val) { b_.putFloat(rawEV_, row, val); }
        void set_rawEW(int row, float val) { b_.putFloat(rawEW_, row, val); }
        void set_recEU(int row, float val) { b_.putFloat(recEU_, row, val); }
        void set_recEV(int row, float val) { b_.putFloat(recEV_, row, val); }
        void set_recEW(int row, float val) { b_.putFloat(recEW_, row, val); }
        void set_recDTU(int row, float val) { b_.putFloat(recDTU_, row, val); }
        void set_recDTV(int row, float val) { b_.putFloat(recDTV_, row, val); }
        void set_recDTW(int row, float val) { b_.putFloat(recDTW_, row, val); }
        void set_recFTU(int row, float val) { b_.putFloat(recFTU_, row, val); }
        void set_recFTV(int row, float val) { b_.putFloat(recFTV_, row, val); }
        void set_recFTW(int row, float val) { b_.putFloat(recFTW_, row, val); }
        void set_size(int row, float val) { b_.putFloat(size_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::CaloExtras", 300, 46);
            s.parse("dbstU/S,dbstV/S,dbstW/S,rawEU/F,rawEV/F,rawEW/F,recEU/F,recEV/F,recEW/F,recDTU/F,recDTV/F,recDTW/F,recFTU/F,recFTV/F,recFTW/F,size/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int dbstU_;
        const int dbstV_;
        const int dbstW_;
        const int rawEU_;
        const int rawEV_;
        const int rawEW_;
        const int recEU_;
        const int recEV_;
        const int recEW_;
        const int recDTU_;
        const int recDTV_;
        const int recDTW_;
        const int recFTU_;
        const int recFTV_;
        const int recFTW_;
        const int size_;
    };

    /// Track information for Particles bank
    class Track {
    public:
        explicit Track(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            status_(b.getSchema().getEntryOrder("status")),
            q_(b.getSchema().getEntryOrder("q")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            NDF_(b.getSchema().getEntryOrder("NDF")),
            hbindex_(b.getSchema().getEntryOrder("hbindex"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the track in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// status of the track
        int status(int row) const noexcept { return b_.getInt(status_, row); }
        /// charge of the track
        int q(int row) const noexcept { return b_.getInt(q_, row); }
        /// Chi2 (or quality) track fitting
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// number of degrees of freedom in track fitting
        int NDF(int row) const noexcept { return b_.getInt(NDF_, row); }
        /// row number of corresponding track in RECHB::Track
        int hbindex(int row) const noexcept { return b_.getInt(hbindex_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }
        void set_q(int row, int8_t val) { b_.putByte(q_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_NDF(int row, int16_t val) { b_.putShort(NDF_, row, val); }
        void set_hbindex(int row, int16_t val) { b_.putShort(hbindex_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::Track", 300, 36);
            s.parse("index/S,pindex/S,detector/B,sector/B,status/S,q/B,chi2/F,NDF/S,hbindex/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int status_;
        const int q_;
        const int chi2_;
        const int NDF_;
        const int hbindex_;
    };

    /// Track information for Particles bank
    class UTrack {
    public:
        explicit UTrack(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            status_(b.getSchema().getEntryOrder("status")),
            q_(b.getSchema().getEntryOrder("q")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            NDF_(b.getSchema().getEntryOrder("NDF")),
            hbindex_(b.getSchema().getEntryOrder("hbindex")),
            px_(b.getSchema().getEntryOrder("px")),
            py_(b.getSchema().getEntryOrder("py")),
            pz_(b.getSchema().getEntryOrder("pz")),
            vx_(b.getSchema().getEntryOrder("vx")),
            vy_(b.getSchema().getEntryOrder("vy")),
            vz_(b.getSchema().getEntryOrder("vz"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the track in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// status of the track
        int status(int row) const noexcept { return b_.getInt(status_, row); }
        /// charge of the track
        int q(int row) const noexcept { return b_.getInt(q_, row); }
        /// Chi2 (or quality) track fitting
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// number of degrees of freedom in track fitting
        int NDF(int row) const noexcept { return b_.getInt(NDF_, row); }
        /// row number of corresponding track in RECHB::Track
        int hbindex(int row) const noexcept { return b_.getInt(hbindex_, row); }
        /// x component of the momentum (GeV)
        float px(int row) const noexcept { return b_.getFloat(px_, row); }
        /// y component of the momentum (GeV)
        float py(int row) const noexcept { return b_.getFloat(py_, row); }
        /// z component of the momentum (GeV)
        float pz(int row) const noexcept { return b_.getFloat(pz_, row); }
        /// x component of the vertex (cm)
        float vx(int row) const noexcept { return b_.getFloat(vx_, row); }
        /// y component of the vertex (cm)
        float vy(int row) const noexcept { return b_.getFloat(vy_, row); }
        /// z component of the vertex (cm)
        float vz(int row) const noexcept { return b_.getFloat(vz_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }
        void set_q(int row, int8_t val) { b_.putByte(q_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_NDF(int row, int16_t val) { b_.putShort(NDF_, row, val); }
        void set_hbindex(int row, int16_t val) { b_.putShort(hbindex_, row, val); }
        void set_px(int row, float val) { b_.putFloat(px_, row, val); }
        void set_py(int row, float val) { b_.putFloat(py_, row, val); }
        void set_pz(int row, float val) { b_.putFloat(pz_, row, val); }
        void set_vx(int row, float val) { b_.putFloat(vx_, row, val); }
        void set_vy(int row, float val) { b_.putFloat(vy_, row, val); }
        void set_vz(int row, float val) { b_.putFloat(vz_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::UTrack", 300, 44);
            s.parse("index/S,pindex/S,detector/B,sector/B,status/S,q/B,chi2/F,NDF/S,hbindex/S,px/F,py/F,pz/F,vx/F,vy/F,vz/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int status_;
        const int q_;
        const int chi2_;
        const int NDF_;
        const int hbindex_;
        const int px_;
        const int py_;
        const int pz_;
        const int vx_;
        const int vy_;
        const int vz_;
    };

    /// Track information for Particles bank
    class FTrack {
    public:
        explicit FTrack(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            status_(b.getSchema().getEntryOrder("status")),
            q_(b.getSchema().getEntryOrder("q")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            NDF_(b.getSchema().getEntryOrder("NDF")),
            hbindex_(b.getSchema().getEntryOrder("hbindex")),
            px_(b.getSchema().getEntryOrder("px")),
            py_(b.getSchema().getEntryOrder("py")),
            pz_(b.getSchema().getEntryOrder("pz")),
            vx_(b.getSchema().getEntryOrder("vx")),
            vy_(b.getSchema().getEntryOrder("vy")),
            vz_(b.getSchema().getEntryOrder("vz"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the track in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// status of the track
        int status(int row) const noexcept { return b_.getInt(status_, row); }
        /// charge of the track
        int q(int row) const noexcept { return b_.getInt(q_, row); }
        /// Chi2 (or quality) track fitting
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// number of degrees of freedom in track fitting
        int NDF(int row) const noexcept { return b_.getInt(NDF_, row); }
        /// row number of corresponding track in RECHB::Track
        int hbindex(int row) const noexcept { return b_.getInt(hbindex_, row); }
        /// x component of the momentum (GeV)
        float px(int row) const noexcept { return b_.getFloat(px_, row); }
        /// y component of the momentum (GeV)
        float py(int row) const noexcept { return b_.getFloat(py_, row); }
        /// z component of the momentum (GeV)
        float pz(int row) const noexcept { return b_.getFloat(pz_, row); }
        /// x component of the vertex (cm)
        float vx(int row) const noexcept { return b_.getFloat(vx_, row); }
        /// y component of the vertex (cm)
        float vy(int row) const noexcept { return b_.getFloat(vy_, row); }
        /// z component of the vertex (cm)
        float vz(int row) const noexcept { return b_.getFloat(vz_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }
        void set_q(int row, int8_t val) { b_.putByte(q_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_NDF(int row, int16_t val) { b_.putShort(NDF_, row, val); }
        void set_hbindex(int row, int16_t val) { b_.putShort(hbindex_, row, val); }
        void set_px(int row, float val) { b_.putFloat(px_, row, val); }
        void set_py(int row, float val) { b_.putFloat(py_, row, val); }
        void set_pz(int row, float val) { b_.putFloat(pz_, row, val); }
        void set_vx(int row, float val) { b_.putFloat(vx_, row, val); }
        void set_vy(int row, float val) { b_.putFloat(vy_, row, val); }
        void set_vz(int row, float val) { b_.putFloat(vz_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::FTrack", 300, 47);
            s.parse("index/S,pindex/S,detector/B,sector/B,status/S,q/B,chi2/F,NDF/S,hbindex/S,px/F,py/F,pz/F,vx/F,vy/F,vz/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int status_;
        const int q_;
        const int chi2_;
        const int NDF_;
        const int hbindex_;
        const int px_;
        const int py_;
        const int pz_;
        const int vx_;
        const int vy_;
        const int vz_;
    };

    /// Track Cross information for Particles bank (deprecated by REC::Traj)
    class TrackCross {
    public:
        explicit TrackCross(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            c_x_(b.getSchema().getEntryOrder("c_x")),
            c_y_(b.getSchema().getEntryOrder("c_y")),
            c_z_(b.getSchema().getEntryOrder("c_z")),
            c_ux_(b.getSchema().getEntryOrder("c_ux")),
            c_uy_(b.getSchema().getEntryOrder("c_uy")),
            c_uz_(b.getSchema().getEntryOrder("c_uz")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the cross in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the track bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// layer of the track
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Cross x-position in the lab (in cm)
        float c_x(int row) const noexcept { return b_.getFloat(c_x_, row); }
        /// Cross y-position in the lab (in cm)
        float c_y(int row) const noexcept { return b_.getFloat(c_y_, row); }
        /// Cross z-position in the lab (in cm)
        float c_z(int row) const noexcept { return b_.getFloat(c_z_, row); }
        /// Cross unit x-direction vector in the lab
        float c_ux(int row) const noexcept { return b_.getFloat(c_ux_, row); }
        /// Cross unit y-direction vector in the lab
        float c_uy(int row) const noexcept { return b_.getFloat(c_uy_, row); }
        /// Cross unit z-direction vector in the lab
        float c_uz(int row) const noexcept { return b_.getFloat(c_uz_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_c_x(int row, float val) { b_.putFloat(c_x_, row, val); }
        void set_c_y(int row, float val) { b_.putFloat(c_y_, row, val); }
        void set_c_z(int row, float val) { b_.putFloat(c_z_, row, val); }
        void set_c_ux(int row, float val) { b_.putFloat(c_ux_, row, val); }
        void set_c_uy(int row, float val) { b_.putFloat(c_uy_, row, val); }
        void set_c_uz(int row, float val) { b_.putFloat(c_uz_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::TrackCross", 300, 37);
            s.parse("index/S,pindex/S,detector/B,sector/B,layer/B,c_x/F,c_y/F,c_z/F,c_ux/F,c_uy/F,c_uz/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int layer_;
        const int c_x_;
        const int c_y_;
        const int c_z_;
        const int c_ux_;
        const int c_uy_;
        const int c_uz_;
        const int status_;
    };

    /// reconstructed track covariance matrix
    class CovMat {
    public:
        explicit CovMat(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            C11_(b.getSchema().getEntryOrder("C11")),
            C12_(b.getSchema().getEntryOrder("C12")),
            C13_(b.getSchema().getEntryOrder("C13")),
            C14_(b.getSchema().getEntryOrder("C14")),
            C15_(b.getSchema().getEntryOrder("C15")),
            C16_(b.getSchema().getEntryOrder("C16")),
            C22_(b.getSchema().getEntryOrder("C22")),
            C23_(b.getSchema().getEntryOrder("C23")),
            C24_(b.getSchema().getEntryOrder("C24")),
            C25_(b.getSchema().getEntryOrder("C25")),
            C26_(b.getSchema().getEntryOrder("C26")),
            C33_(b.getSchema().getEntryOrder("C33")),
            C34_(b.getSchema().getEntryOrder("C34")),
            C35_(b.getSchema().getEntryOrder("C35")),
            C36_(b.getSchema().getEntryOrder("C36")),
            C44_(b.getSchema().getEntryOrder("C44")),
            C45_(b.getSchema().getEntryOrder("C45")),
            C46_(b.getSchema().getEntryOrder("C46")),
            C55_(b.getSchema().getEntryOrder("C55")),
            C56_(b.getSchema().getEntryOrder("C56")),
            C66_(b.getSchema().getEntryOrder("C66"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the track in Tracks bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// var(x) (cm^2)
        float C11(int row) const noexcept { return b_.getFloat(C11_, row); }
        /// cov(x,y)
        float C12(int row) const noexcept { return b_.getFloat(C12_, row); }
        /// cov(x,z)
        float C13(int row) const noexcept { return b_.getFloat(C13_, row); }
        /// cov(x,theta)
        float C14(int row) const noexcept { return b_.getFloat(C14_, row); }
        /// cov(x,phi)
        float C15(int row) const noexcept { return b_.getFloat(C15_, row); }
        /// cov(x,p)
        float C16(int row) const noexcept { return b_.getFloat(C16_, row); }
        /// var(y) (cm^2)
        float C22(int row) const noexcept { return b_.getFloat(C22_, row); }
        /// cov(y,z)
        float C23(int row) const noexcept { return b_.getFloat(C23_, row); }
        /// cov(y,theta)
        float C24(int row) const noexcept { return b_.getFloat(C24_, row); }
        /// cov(y,phi)
        float C25(int row) const noexcept { return b_.getFloat(C25_, row); }
        /// cov(y,p)
        float C26(int row) const noexcept { return b_.getFloat(C26_, row); }
        /// var(z) (cm^2)
        float C33(int row) const noexcept { return b_.getFloat(C33_, row); }
        /// cov(z,theta)
        float C34(int row) const noexcept { return b_.getFloat(C34_, row); }
        /// cov(z,phi)
        float C35(int row) const noexcept { return b_.getFloat(C35_, row); }
        /// cov(z,p)
        float C36(int row) const noexcept { return b_.getFloat(C36_, row); }
        /// var(theta) (rad^2)
        float C44(int row) const noexcept { return b_.getFloat(C44_, row); }
        /// cov(theta,phi)
        float C45(int row) const noexcept { return b_.getFloat(C45_, row); }
        /// cov(theta,p)
        float C46(int row) const noexcept { return b_.getFloat(C46_, row); }
        /// var(phi) (rad^2)
        float C55(int row) const noexcept { return b_.getFloat(C55_, row); }
        /// cov(phi,p)
        float C56(int row) const noexcept { return b_.getFloat(C56_, row); }
        /// var(p) (GeV^2)
        float C66(int row) const noexcept { return b_.getFloat(C66_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_C11(int row, float val) { b_.putFloat(C11_, row, val); }
        void set_C12(int row, float val) { b_.putFloat(C12_, row, val); }
        void set_C13(int row, float val) { b_.putFloat(C13_, row, val); }
        void set_C14(int row, float val) { b_.putFloat(C14_, row, val); }
        void set_C15(int row, float val) { b_.putFloat(C15_, row, val); }
        void set_C16(int row, float val) { b_.putFloat(C16_, row, val); }
        void set_C22(int row, float val) { b_.putFloat(C22_, row, val); }
        void set_C23(int row, float val) { b_.putFloat(C23_, row, val); }
        void set_C24(int row, float val) { b_.putFloat(C24_, row, val); }
        void set_C25(int row, float val) { b_.putFloat(C25_, row, val); }
        void set_C26(int row, float val) { b_.putFloat(C26_, row, val); }
        void set_C33(int row, float val) { b_.putFloat(C33_, row, val); }
        void set_C34(int row, float val) { b_.putFloat(C34_, row, val); }
        void set_C35(int row, float val) { b_.putFloat(C35_, row, val); }
        void set_C36(int row, float val) { b_.putFloat(C36_, row, val); }
        void set_C44(int row, float val) { b_.putFloat(C44_, row, val); }
        void set_C45(int row, float val) { b_.putFloat(C45_, row, val); }
        void set_C46(int row, float val) { b_.putFloat(C46_, row, val); }
        void set_C55(int row, float val) { b_.putFloat(C55_, row, val); }
        void set_C56(int row, float val) { b_.putFloat(C56_, row, val); }
        void set_C66(int row, float val) { b_.putFloat(C66_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::CovMat", 300, 38);
            s.parse("index/S,pindex/S,C11/F,C12/F,C13/F,C14/F,C15/F,C16/F,C22/F,C23/F,C24/F,C25/F,C26/F,C33/F,C34/F,C35/F,C36/F,C44/F,C45/F,C46/F,C55/F,C56/F,C66/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int C11_;
        const int C12_;
        const int C13_;
        const int C14_;
        const int C15_;
        const int C16_;
        const int C22_;
        const int C23_;
        const int C24_;
        const int C25_;
        const int C26_;
        const int C33_;
        const int C34_;
        const int C35_;
        const int C36_;
        const int C44_;
        const int C45_;
        const int C46_;
        const int C55_;
        const int C56_;
        const int C66_;
    };

    /// Track Cross information for Particles bank
    class VertDoca {
    public:
        explicit VertDoca(hipo::bank& b) : b_(b),
            index1_(b.getSchema().getEntryOrder("index1")),
            index2_(b.getSchema().getEntryOrder("index2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            x1_(b.getSchema().getEntryOrder("x1")),
            y1_(b.getSchema().getEntryOrder("y1")),
            z1_(b.getSchema().getEntryOrder("z1")),
            cx1_(b.getSchema().getEntryOrder("cx1")),
            cy1_(b.getSchema().getEntryOrder("cy1")),
            cz1_(b.getSchema().getEntryOrder("cz1")),
            x2_(b.getSchema().getEntryOrder("x2")),
            y2_(b.getSchema().getEntryOrder("y2")),
            z2_(b.getSchema().getEntryOrder("z2")),
            cx2_(b.getSchema().getEntryOrder("cx2")),
            cy2_(b.getSchema().getEntryOrder("cy2")),
            cz2_(b.getSchema().getEntryOrder("cz2")),
            r_(b.getSchema().getEntryOrder("r"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the first track in Tracks bank
        int index1(int row) const noexcept { return b_.getInt(index1_, row); }
        /// index of the second track in Tracks bank
        int index2(int row) const noexcept { return b_.getInt(index2_, row); }
        /// x-position of the common vertex (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// y-position of the common vertex (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// z-position of the common vertex (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// x-position of the first track at the DOCA point (cm)
        float x1(int row) const noexcept { return b_.getFloat(x1_, row); }
        /// y-position of the first track at the DOCA point (cm)
        float y1(int row) const noexcept { return b_.getFloat(y1_, row); }
        /// z-position of the first track at the DOCA point (cm)
        float z1(int row) const noexcept { return b_.getFloat(z1_, row); }
        /// x-direction vector of the first track at the DOCA point (cm)
        float cx1(int row) const noexcept { return b_.getFloat(cx1_, row); }
        /// y-direction vector of the first track at the DOCA point (cm)
        float cy1(int row) const noexcept { return b_.getFloat(cy1_, row); }
        /// z-direction vector of the first track at the DOCA point (cm)
        float cz1(int row) const noexcept { return b_.getFloat(cz1_, row); }
        /// x-position of the second track at the DOCA point (cm)
        float x2(int row) const noexcept { return b_.getFloat(x2_, row); }
        /// y-position of the second track at the DOCA point (cm)
        float y2(int row) const noexcept { return b_.getFloat(y2_, row); }
        /// z-position of the second track at the DOCA point (cm)
        float z2(int row) const noexcept { return b_.getFloat(z2_, row); }
        /// x-direction vector of the second track at the DOCA point (cm)
        float cx2(int row) const noexcept { return b_.getFloat(cx2_, row); }
        /// y-direction vector of the second track at the DOCA point (cm)
        float cy2(int row) const noexcept { return b_.getFloat(cy2_, row); }
        /// z-direction vector of the second track at the DOCA point (cm)
        float cz2(int row) const noexcept { return b_.getFloat(cz2_, row); }
        /// The distance between two tracks (cm)
        float r(int row) const noexcept { return b_.getFloat(r_, row); }

        void set_index1(int row, int16_t val) { b_.putShort(index1_, row, val); }
        void set_index2(int row, int16_t val) { b_.putShort(index2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_x1(int row, float val) { b_.putFloat(x1_, row, val); }
        void set_y1(int row, float val) { b_.putFloat(y1_, row, val); }
        void set_z1(int row, float val) { b_.putFloat(z1_, row, val); }
        void set_cx1(int row, float val) { b_.putFloat(cx1_, row, val); }
        void set_cy1(int row, float val) { b_.putFloat(cy1_, row, val); }
        void set_cz1(int row, float val) { b_.putFloat(cz1_, row, val); }
        void set_x2(int row, float val) { b_.putFloat(x2_, row, val); }
        void set_y2(int row, float val) { b_.putFloat(y2_, row, val); }
        void set_z2(int row, float val) { b_.putFloat(z2_, row, val); }
        void set_cx2(int row, float val) { b_.putFloat(cx2_, row, val); }
        void set_cy2(int row, float val) { b_.putFloat(cy2_, row, val); }
        void set_cz2(int row, float val) { b_.putFloat(cz2_, row, val); }
        void set_r(int row, float val) { b_.putFloat(r_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::VertDoca", 300, 39);
            s.parse("index1/S,index2/S,x/F,y/F,z/F,x1/F,y1/F,z1/F,cx1/F,cy1/F,cz1/F,x2/F,y2/F,z2/F,cx2/F,cy2/F,cz2/F,r/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index1_;
        const int index2_;
        const int x_;
        const int y_;
        const int z_;
        const int x1_;
        const int y1_;
        const int z1_;
        const int cx1_;
        const int cy1_;
        const int cz1_;
        const int x2_;
        const int y2_;
        const int z2_;
        const int cx2_;
        const int cy2_;
        const int cz2_;
        const int r_;
    };

    /// Trajectory information for Particles bank
    class Traj {
    public:
        explicit Traj(hipo::bank& b) : b_(b),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            index_(b.getSchema().getEntryOrder("index")),
            detector_(b.getSchema().getEntryOrder("detector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            cx_(b.getSchema().getEntryOrder("cx")),
            cy_(b.getSchema().getEntryOrder("cy")),
            cz_(b.getSchema().getEntryOrder("cz")),
            path_(b.getSchema().getEntryOrder("path")),
            edge_(b.getSchema().getEntryOrder("edge"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the particle in REC::Particle
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// index of the track in its tracking bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// DetectorID as defined in org.jlab.detector.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Layer ID as defined in org.jlab.detector.DetectorLayer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// x-position of the track at the detector surface (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// y-position of the track at the detector surface (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// z-position of the track at the detector surface (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// direction cosline of the track at the detector surface
        float cx(int row) const noexcept { return b_.getFloat(cx_, row); }
        /// direction cosline of the track at the detector surface
        float cy(int row) const noexcept { return b_.getFloat(cy_, row); }
        /// direction cosline of the track at the detector surface
        float cz(int row) const noexcept { return b_.getFloat(cz_, row); }
        /// path length from primary vertex to the detector surface (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// signed distance between track position and closest detector edge (cm)
        float edge(int row) const noexcept { return b_.getFloat(edge_, row); }

        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_cx(int row, float val) { b_.putFloat(cx_, row, val); }
        void set_cy(int row, float val) { b_.putFloat(cy_, row, val); }
        void set_cz(int row, float val) { b_.putFloat(cz_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_edge(int row, float val) { b_.putFloat(edge_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("REC::Traj", 300, 40);
            s.parse("pindex/S,index/S,detector/B,layer/B,x/F,y/F,z/F,cx/F,cy/F,cz/F,path/F,edge/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int pindex_;
        const int index_;
        const int detector_;
        const int layer_;
        const int x_;
        const int y_;
        const int z_;
        const int cx_;
        const int cy_;
        const int cz_;
        const int path_;
        const int edge_;
    };

} // namespace rec

namespace rechb {

    /// Event Header Bank
    class Event {
    public:
        explicit Event(hipo::bank& b) : b_(b),
            category_(b.getSchema().getEntryOrder("category")),
            topology_(b.getSchema().getEntryOrder("topology")),
            beamCharge_(b.getSchema().getEntryOrder("beamCharge")),
            liveTime_(b.getSchema().getEntryOrder("liveTime")),
            startTime_(b.getSchema().getEntryOrder("startTime")),
            RFTime_(b.getSchema().getEntryOrder("RFTime")),
            helicity_(b.getSchema().getEntryOrder("helicity")),
            helicityRaw_(b.getSchema().getEntryOrder("helicityRaw")),
            procTime_(b.getSchema().getEntryOrder("procTime"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// Undefined
        long category(int row) const noexcept { return b_.getLong(category_, row); }
        /// Undefined
        long topology(int row) const noexcept { return b_.getLong(topology_, row); }
        /// Beam charge, gated (nano-Coulomb)
        float beamCharge(int row) const noexcept { return b_.getFloat(beamCharge_, row); }
        /// Lifetime
        double liveTime(int row) const noexcept { return b_.getDouble(liveTime_, row); }
        /// Event Start Time (ns)
        float startTime(int row) const noexcept { return b_.getFloat(startTime_, row); }
        /// RF Time (ns)
        float RFTime(int row) const noexcept { return b_.getFloat(RFTime_, row); }
        /// Helicity of Event (+/-1, else undefined), with HWP-correction
        int helicity(int row) const noexcept { return b_.getInt(helicity_, row); }
        /// Helicity of Event (+/-1, else undefined)
        int helicityRaw(int row) const noexcept { return b_.getInt(helicityRaw_, row); }
        /// Event Processing Time (UNIX Time = seconds)
        float procTime(int row) const noexcept { return b_.getFloat(procTime_, row); }

        void set_category(int row, int64_t val) { b_.putLong(category_, row, val); }
        void set_topology(int row, int64_t val) { b_.putLong(topology_, row, val); }
        void set_beamCharge(int row, float val) { b_.putFloat(beamCharge_, row, val); }
        void set_liveTime(int row, double val) { b_.putDouble(liveTime_, row, val); }
        void set_startTime(int row, float val) { b_.putFloat(startTime_, row, val); }
        void set_RFTime(int row, float val) { b_.putFloat(RFTime_, row, val); }
        void set_helicity(int row, int8_t val) { b_.putByte(helicity_, row, val); }
        void set_helicityRaw(int row, int8_t val) { b_.putByte(helicityRaw_, row, val); }
        void set_procTime(int row, float val) { b_.putFloat(procTime_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::Event", 300, 10);
            s.parse("category/L,topology/L,beamCharge/F,liveTime/D,startTime/F,RFTime/F,helicity/B,helicityRaw/B,procTime/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int category_;
        const int topology_;
        const int beamCharge_;
        const int liveTime_;
        const int startTime_;
        const int RFTime_;
        const int helicity_;
        const int helicityRaw_;
        const int procTime_;
    };

    /// Reconstructed Particle Information
    class Particle {
    public:
        explicit Particle(hipo::bank& b) : b_(b),
            pid_(b.getSchema().getEntryOrder("pid")),
            px_(b.getSchema().getEntryOrder("px")),
            py_(b.getSchema().getEntryOrder("py")),
            pz_(b.getSchema().getEntryOrder("pz")),
            vx_(b.getSchema().getEntryOrder("vx")),
            vy_(b.getSchema().getEntryOrder("vy")),
            vz_(b.getSchema().getEntryOrder("vz")),
            vt_(b.getSchema().getEntryOrder("vt")),
            charge_(b.getSchema().getEntryOrder("charge")),
            beta_(b.getSchema().getEntryOrder("beta")),
            chi2pid_(b.getSchema().getEntryOrder("chi2pid")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// particle id in LUND conventions
        int pid(int row) const noexcept { return b_.getInt(pid_, row); }
        /// x component of the momentum (GeV)
        float px(int row) const noexcept { return b_.getFloat(px_, row); }
        /// y component of the momentum (GeV)
        float py(int row) const noexcept { return b_.getFloat(py_, row); }
        /// z component of the momentum (GeV)
        float pz(int row) const noexcept { return b_.getFloat(pz_, row); }
        /// x component of the vertex (cm)
        float vx(int row) const noexcept { return b_.getFloat(vx_, row); }
        /// y component of the vertex (cm)
        float vy(int row) const noexcept { return b_.getFloat(vy_, row); }
        /// z component of the vertex (cm)
        float vz(int row) const noexcept { return b_.getFloat(vz_, row); }
        /// RF and z corrected vertex time (ns)
        float vt(int row) const noexcept { return b_.getFloat(vt_, row); }
        /// particle charge
        int charge(int row) const noexcept { return b_.getInt(charge_, row); }
        /// particle beta measured by TOF
        float beta(int row) const noexcept { return b_.getFloat(beta_, row); }
        /// Chi2 of assigned PID
        float chi2pid(int row) const noexcept { return b_.getFloat(chi2pid_, row); }
        /// particle status (represents detector collection it passed)
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_pid(int row, int32_t val) { b_.putInt(pid_, row, val); }
        void set_px(int row, float val) { b_.putFloat(px_, row, val); }
        void set_py(int row, float val) { b_.putFloat(py_, row, val); }
        void set_pz(int row, float val) { b_.putFloat(pz_, row, val); }
        void set_vx(int row, float val) { b_.putFloat(vx_, row, val); }
        void set_vy(int row, float val) { b_.putFloat(vy_, row, val); }
        void set_vz(int row, float val) { b_.putFloat(vz_, row, val); }
        void set_vt(int row, float val) { b_.putFloat(vt_, row, val); }
        void set_charge(int row, int8_t val) { b_.putByte(charge_, row, val); }
        void set_beta(int row, float val) { b_.putFloat(beta_, row, val); }
        void set_chi2pid(int row, float val) { b_.putFloat(chi2pid_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::Particle", 300, 11);
            s.parse("pid/I,px/F,py/F,pz/F,vx/F,vy/F,vz/F,vt/F,charge/B,beta/F,chi2pid/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int pid_;
        const int px_;
        const int py_;
        const int pz_;
        const int vx_;
        const int vy_;
        const int vz_;
        const int vt_;
        const int charge_;
        const int beta_;
        const int chi2pid_;
        const int status_;
    };

    /// Calorimeter Responses for Particles bank
    class Calorimeter {
    public:
        explicit Calorimeter(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            energy_(b.getSchema().getEntryOrder("energy")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            hx_(b.getSchema().getEntryOrder("hx")),
            hy_(b.getSchema().getEntryOrder("hy")),
            hz_(b.getSchema().getEntryOrder("hz")),
            lu_(b.getSchema().getEntryOrder("lu")),
            lv_(b.getSchema().getEntryOrder("lv")),
            lw_(b.getSchema().getEntryOrder("lw")),
            du_(b.getSchema().getEntryOrder("du")),
            dv_(b.getSchema().getEntryOrder("dv")),
            dw_(b.getSchema().getEntryOrder("dw")),
            m2u_(b.getSchema().getEntryOrder("m2u")),
            m2v_(b.getSchema().getEntryOrder("m2v")),
            m2w_(b.getSchema().getEntryOrder("m2w")),
            m3u_(b.getSchema().getEntryOrder("m3u")),
            m3v_(b.getSchema().getEntryOrder("m3v")),
            m3w_(b.getSchema().getEntryOrder("m3w")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the hit in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the Detector hit
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// Layer ID, as defined in org.jlab.detector.base.DetectorLayer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Energy associated with the hit (GeV)
        float energy(int row) const noexcept { return b_.getFloat(energy_, row); }
        /// Time associated with the hit (ns)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the hit position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of hit-track matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the hit (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the hit (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the hit (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// X coordinate of the matched hit (cm)
        float hx(int row) const noexcept { return b_.getFloat(hx_, row); }
        /// Y coordinate of the mathced hit (cm)
        float hy(int row) const noexcept { return b_.getFloat(hy_, row); }
        /// Z coordinate of the matched hit (cm)
        float hz(int row) const noexcept { return b_.getFloat(hz_, row); }
        /// distance on U-side
        float lu(int row) const noexcept { return b_.getFloat(lu_, row); }
        /// distance on V-side
        float lv(int row) const noexcept { return b_.getFloat(lv_, row); }
        /// distance on W-side
        float lw(int row) const noexcept { return b_.getFloat(lw_, row); }
        /// shower width on U-side
        float du(int row) const noexcept { return b_.getFloat(du_, row); }
        /// shower width on V-side
        float dv(int row) const noexcept { return b_.getFloat(dv_, row); }
        /// shower width on w-side
        float dw(int row) const noexcept { return b_.getFloat(dw_, row); }
        /// 2nd moment of the shower on U-side
        float m2u(int row) const noexcept { return b_.getFloat(m2u_, row); }
        /// 2nd moment of the shower on V-side
        float m2v(int row) const noexcept { return b_.getFloat(m2v_, row); }
        /// 2nd moment of the shower on W-side
        float m2w(int row) const noexcept { return b_.getFloat(m2w_, row); }
        /// 3rd moment of the shower on U-side
        float m3u(int row) const noexcept { return b_.getFloat(m3u_, row); }
        /// 3rd moment of the shower on V-side
        float m3v(int row) const noexcept { return b_.getFloat(m3v_, row); }
        /// 3rd moment of the shower on W-side
        float m3w(int row) const noexcept { return b_.getFloat(m3w_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_energy(int row, float val) { b_.putFloat(energy_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_hx(int row, float val) { b_.putFloat(hx_, row, val); }
        void set_hy(int row, float val) { b_.putFloat(hy_, row, val); }
        void set_hz(int row, float val) { b_.putFloat(hz_, row, val); }
        void set_lu(int row, float val) { b_.putFloat(lu_, row, val); }
        void set_lv(int row, float val) { b_.putFloat(lv_, row, val); }
        void set_lw(int row, float val) { b_.putFloat(lw_, row, val); }
        void set_du(int row, float val) { b_.putFloat(du_, row, val); }
        void set_dv(int row, float val) { b_.putFloat(dv_, row, val); }
        void set_dw(int row, float val) { b_.putFloat(dw_, row, val); }
        void set_m2u(int row, float val) { b_.putFloat(m2u_, row, val); }
        void set_m2v(int row, float val) { b_.putFloat(m2v_, row, val); }
        void set_m2w(int row, float val) { b_.putFloat(m2w_, row, val); }
        void set_m3u(int row, float val) { b_.putFloat(m3u_, row, val); }
        void set_m3v(int row, float val) { b_.putFloat(m3v_, row, val); }
        void set_m3w(int row, float val) { b_.putFloat(m3w_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::Calorimeter", 300, 12);
            s.parse("index/S,pindex/S,detector/B,sector/B,layer/B,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,hx/F,hy/F,hz/F,lu/F,lv/F,lw/F,du/F,dv/F,dw/F,m2u/F,m2v/F,m2w/F,m3u/F,m3v/F,m3w/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int layer_;
        const int energy_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int hx_;
        const int hy_;
        const int hz_;
        const int lu_;
        const int lv_;
        const int lw_;
        const int du_;
        const int dv_;
        const int dw_;
        const int m2u_;
        const int m2v_;
        const int m2w_;
        const int m3u_;
        const int m3v_;
        const int m3w_;
        const int status_;
    };

    /// Cherenkov Responses for Particles bank
    class Cherenkov {
    public:
        explicit Cherenkov(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            nphe_(b.getSchema().getEntryOrder("nphe")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            dtheta_(b.getSchema().getEntryOrder("dtheta")),
            dphi_(b.getSchema().getEntryOrder("dphi")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the hit in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the Detector hit
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// Number of photoelectrons from Cherenkov radiation
        float nphe(int row) const noexcept { return b_.getFloat(nphe_, row); }
        /// Time associated with the hit (ns)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the hit position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of hit-track matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the hit (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the hit (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the hit (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// Expected Theta Resolution (deg)
        float dtheta(int row) const noexcept { return b_.getFloat(dtheta_, row); }
        /// Expected Phi Resolution (deg)
        float dphi(int row) const noexcept { return b_.getFloat(dphi_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_nphe(int row, float val) { b_.putFloat(nphe_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_dtheta(int row, float val) { b_.putFloat(dtheta_, row, val); }
        void set_dphi(int row, float val) { b_.putFloat(dphi_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::Cherenkov", 300, 13);
            s.parse("index/S,pindex/S,detector/B,sector/B,nphe/F,time/F,path/F,chi2/F,x/F,y/F,z/F,dtheta/F,dphi/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int nphe_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int dtheta_;
        const int dphi_;
        const int status_;
    };

    /// Forward Tagger information for Particles bank
    class ForwardTagger {
    public:
        explicit ForwardTagger(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            energy_(b.getSchema().getEntryOrder("energy")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            dx_(b.getSchema().getEntryOrder("dx")),
            dy_(b.getSchema().getEntryOrder("dy")),
            radius_(b.getSchema().getEntryOrder("radius")),
            size_(b.getSchema().getEntryOrder("size")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the cluster in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// layer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Energy associated with the cluster (GeV)
        float energy(int row) const noexcept { return b_.getFloat(energy_, row); }
        /// Time associated with the cluster (cm)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the cluster position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of cluster-particle matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the cluster
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the cluster
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the cluster
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// Cluster width in x
        float dx(int row) const noexcept { return b_.getFloat(dx_, row); }
        /// Cluster width in y
        float dy(int row) const noexcept { return b_.getFloat(dy_, row); }
        /// Cluster radius
        float radius(int row) const noexcept { return b_.getFloat(radius_, row); }
        /// size of the cluster
        int size(int row) const noexcept { return b_.getInt(size_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_energy(int row, float val) { b_.putFloat(energy_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_dx(int row, float val) { b_.putFloat(dx_, row, val); }
        void set_dy(int row, float val) { b_.putFloat(dy_, row, val); }
        void set_radius(int row, float val) { b_.putFloat(radius_, row, val); }
        void set_size(int row, int16_t val) { b_.putShort(size_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::ForwardTagger", 300, 14);
            s.parse("index/S,pindex/S,detector/B,layer/B,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,dx/F,dy/F,radius/F,size/S,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int layer_;
        const int energy_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int dx_;
        const int dy_;
        const int radius_;
        const int size_;
        const int status_;
    };

    /// Scintillator Responses for Particles bank
    class Scintillator {
    public:
        explicit Scintillator(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            component_(b.getSchema().getEntryOrder("component")),
            energy_(b.getSchema().getEntryOrder("energy")),
            time_(b.getSchema().getEntryOrder("time")),
            path_(b.getSchema().getEntryOrder("path")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            hx_(b.getSchema().getEntryOrder("hx")),
            hy_(b.getSchema().getEntryOrder("hy")),
            hz_(b.getSchema().getEntryOrder("hz")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the hit in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the Detector hit
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// Layer ID, as defined in org.jlab.detector.base.DetectorLayer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Component of the Detector hit
        int component(int row) const noexcept { return b_.getInt(component_, row); }
        /// Energy associated with the hit (MeV)
        float energy(int row) const noexcept { return b_.getFloat(energy_, row); }
        /// Time associated with the hit (ns)
        float time(int row) const noexcept { return b_.getFloat(time_, row); }
        /// Path from vertex to the hit position (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// Chi2 (or quality) of hit-track matching
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// X coordinate of the hit (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// Y coordinate of the hit (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// Z coordinate of the hit (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// X coordinate of the matched hit (cm)
        float hx(int row) const noexcept { return b_.getFloat(hx_, row); }
        /// Y coordinate of the mathced hit (cm)
        float hy(int row) const noexcept { return b_.getFloat(hy_, row); }
        /// Z coordinate of the matched hit (cm)
        float hz(int row) const noexcept { return b_.getFloat(hz_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_component(int row, int16_t val) { b_.putShort(component_, row, val); }
        void set_energy(int row, float val) { b_.putFloat(energy_, row, val); }
        void set_time(int row, float val) { b_.putFloat(time_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_hx(int row, float val) { b_.putFloat(hx_, row, val); }
        void set_hy(int row, float val) { b_.putFloat(hy_, row, val); }
        void set_hz(int row, float val) { b_.putFloat(hz_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::Scintillator", 300, 15);
            s.parse("index/S,pindex/S,detector/B,sector/B,layer/B,component/S,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,hx/F,hy/F,hz/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int layer_;
        const int component_;
        const int energy_;
        const int time_;
        const int path_;
        const int chi2_;
        const int x_;
        const int y_;
        const int z_;
        const int hx_;
        const int hy_;
        const int hz_;
        const int status_;
    };

    /// Scintillator Cluster Parameters
    class ScintExtras {
    public:
        explicit ScintExtras(hipo::bank& b) : b_(b),
            dedx_(b.getSchema().getEntryOrder("dedx")),
            size_(b.getSchema().getEntryOrder("size")),
            layermult_(b.getSchema().getEntryOrder("layermult"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// Energy deposition per distance travelled (MeV/cm)
        float dedx(int row) const noexcept { return b_.getFloat(dedx_, row); }
        /// cluster size
        int size(int row) const noexcept { return b_.getInt(size_, row); }
        /// cluster layer multiplicity
        int layermult(int row) const noexcept { return b_.getInt(layermult_, row); }

        void set_dedx(int row, float val) { b_.putFloat(dedx_, row, val); }
        void set_size(int row, int16_t val) { b_.putShort(size_, row, val); }
        void set_layermult(int row, int8_t val) { b_.putByte(layermult_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::ScintExtras", 300, 23);
            s.parse("dedx/F,size/S,layermult/B");
            return s;
        }

    private:
        hipo::bank& b_;
        const int dedx_;
        const int size_;
        const int layermult_;
    };

    /// Calorimeter Cluster Parameters
    class CaloExtras {
    public:
        explicit CaloExtras(hipo::bank& b) : b_(b),
            dbstU_(b.getSchema().getEntryOrder("dbstU")),
            dbstV_(b.getSchema().getEntryOrder("dbstV")),
            dbstW_(b.getSchema().getEntryOrder("dbstW")),
            rawEU_(b.getSchema().getEntryOrder("rawEU")),
            rawEV_(b.getSchema().getEntryOrder("rawEV")),
            rawEW_(b.getSchema().getEntryOrder("rawEW")),
            recEU_(b.getSchema().getEntryOrder("recEU")),
            recEV_(b.getSchema().getEntryOrder("recEV")),
            recEW_(b.getSchema().getEntryOrder("recEW")),
            recDTU_(b.getSchema().getEntryOrder("recDTU")),
            recDTV_(b.getSchema().getEntryOrder("recDTV")),
            recDTW_(b.getSchema().getEntryOrder("recDTW")),
            recFTU_(b.getSchema().getEntryOrder("recFTU")),
            recFTV_(b.getSchema().getEntryOrder("recFTV")),
            recFTW_(b.getSchema().getEntryOrder("recFTW")),
            size_(b.getSchema().getEntryOrder("size"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// DB status + U strip*10
        int dbstU(int row) const noexcept { return b_.getInt(dbstU_, row); }
        /// DB status + V strip*10
        int dbstV(int row) const noexcept { return b_.getInt(dbstV_, row); }
        /// DB status + W strip*10
        int dbstW(int row) const noexcept { return b_.getInt(dbstW_, row); }
        /// raw U peak energy
        float rawEU(int row) const noexcept { return b_.getFloat(rawEU_, row); }
        /// raw V peak energy
        float rawEV(int row) const noexcept { return b_.getFloat(rawEV_, row); }
        /// raw W peak energy
        float rawEW(int row) const noexcept { return b_.getFloat(rawEW_, row); }
        /// recon U peak energy (GeV)
        float recEU(int row) const noexcept { return b_.getFloat(recEU_, row); }
        /// recon V peak energy (GeV)
        float recEV(int row) const noexcept { return b_.getFloat(recEV_, row); }
        /// recon W peak energy (GeV)
        float recEW(int row) const noexcept { return b_.getFloat(recEW_, row); }
        /// recon U peak TDC time (ns)
        float recDTU(int row) const noexcept { return b_.getFloat(recDTU_, row); }
        /// recon V peak TDC time (ns)
        float recDTV(int row) const noexcept { return b_.getFloat(recDTV_, row); }
        /// recon W peak TDC time (ns)
        float recDTW(int row) const noexcept { return b_.getFloat(recDTW_, row); }
        /// recon U peak FADC time (ns)
        float recFTU(int row) const noexcept { return b_.getFloat(recFTU_, row); }
        /// recon V peak FADC time (ns)
        float recFTV(int row) const noexcept { return b_.getFloat(recFTV_, row); }
        /// recon W peak FADC time (ns)
        float recFTW(int row) const noexcept { return b_.getFloat(recFTW_, row); }
        /// cluster size
        float size(int row) const noexcept { return b_.getFloat(size_, row); }

        void set_dbstU(int row, int16_t val) { b_.putShort(dbstU_, row, val); }
        void set_dbstV(int row, int16_t val) { b_.putShort(dbstV_, row, val); }
        void set_dbstW(int row, int16_t val) { b_.putShort(dbstW_, row, val); }
        void set_rawEU(int row, float val) { b_.putFloat(rawEU_, row, val); }
        void set_rawEV(int row, float val) { b_.putFloat(rawEV_, row, val); }
        void set_rawEW(int row, float val) { b_.putFloat(rawEW_, row, val); }
        void set_recEU(int row, float val) { b_.putFloat(recEU_, row, val); }
        void set_recEV(int row, float val) { b_.putFloat(recEV_, row, val); }
        void set_recEW(int row, float val) { b_.putFloat(recEW_, row, val); }
        void set_recDTU(int row, float val) { b_.putFloat(recDTU_, row, val); }
        void set_recDTV(int row, float val) { b_.putFloat(recDTV_, row, val); }
        void set_recDTW(int row, float val) { b_.putFloat(recDTW_, row, val); }
        void set_recFTU(int row, float val) { b_.putFloat(recFTU_, row, val); }
        void set_recFTV(int row, float val) { b_.putFloat(recFTV_, row, val); }
        void set_recFTW(int row, float val) { b_.putFloat(recFTW_, row, val); }
        void set_size(int row, float val) { b_.putFloat(size_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::CaloExtras", 300, 24);
            s.parse("dbstU/S,dbstV/S,dbstW/S,rawEU/F,rawEV/F,rawEW/F,recEU/F,recEV/F,recEW/F,recDTU/F,recDTV/F,recDTW/F,recFTU/F,recFTV/F,recFTW/F,size/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int dbstU_;
        const int dbstV_;
        const int dbstW_;
        const int rawEU_;
        const int rawEV_;
        const int rawEW_;
        const int recEU_;
        const int recEV_;
        const int recEW_;
        const int recDTU_;
        const int recDTV_;
        const int recDTW_;
        const int recFTU_;
        const int recFTV_;
        const int recFTW_;
        const int size_;
    };

    /// Track information for Particles bank
    class Track {
    public:
        explicit Track(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            status_(b.getSchema().getEntryOrder("status")),
            q_(b.getSchema().getEntryOrder("q")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            NDF_(b.getSchema().getEntryOrder("NDF"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the track in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// status of the track
        int status(int row) const noexcept { return b_.getInt(status_, row); }
        /// charge of the track
        int q(int row) const noexcept { return b_.getInt(q_, row); }
        /// Chi2 (or quality) track fitting
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// number of degrees of freedom in track fitting
        int NDF(int row) const noexcept { return b_.getInt(NDF_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }
        void set_q(int row, int8_t val) { b_.putByte(q_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_NDF(int row, int16_t val) { b_.putShort(NDF_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::Track", 300, 16);
            s.parse("index/S,pindex/S,detector/B,sector/B,status/S,q/B,chi2/F,NDF/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int status_;
        const int q_;
        const int chi2_;
        const int NDF_;
    };

    /// Track Cross information for Particles bank (deprecated by RECHB::Traj)
    class TrackCross {
    public:
        explicit TrackCross(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            c_x_(b.getSchema().getEntryOrder("c_x")),
            c_y_(b.getSchema().getEntryOrder("c_y")),
            c_z_(b.getSchema().getEntryOrder("c_z")),
            c_ux_(b.getSchema().getEntryOrder("c_ux")),
            c_uy_(b.getSchema().getEntryOrder("c_uy")),
            c_uz_(b.getSchema().getEntryOrder("c_uz")),
            status_(b.getSchema().getEntryOrder("status"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the cross in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the track bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// layer of the track
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// Cross x-position in the lab (in cm)
        float c_x(int row) const noexcept { return b_.getFloat(c_x_, row); }
        /// Cross y-position in the lab (in cm)
        float c_y(int row) const noexcept { return b_.getFloat(c_y_, row); }
        /// Cross z-position in the lab (in cm)
        float c_z(int row) const noexcept { return b_.getFloat(c_z_, row); }
        /// Cross unit x-direction vector in the lab
        float c_ux(int row) const noexcept { return b_.getFloat(c_ux_, row); }
        /// Cross unit y-direction vector in the lab
        float c_uy(int row) const noexcept { return b_.getFloat(c_uy_, row); }
        /// Cross unit z-direction vector in the lab
        float c_uz(int row) const noexcept { return b_.getFloat(c_uz_, row); }
        /// hit status
        int status(int row) const noexcept { return b_.getInt(status_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_c_x(int row, float val) { b_.putFloat(c_x_, row, val); }
        void set_c_y(int row, float val) { b_.putFloat(c_y_, row, val); }
        void set_c_z(int row, float val) { b_.putFloat(c_z_, row, val); }
        void set_c_ux(int row, float val) { b_.putFloat(c_ux_, row, val); }
        void set_c_uy(int row, float val) { b_.putFloat(c_uy_, row, val); }
        void set_c_uz(int row, float val) { b_.putFloat(c_uz_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::TrackCross", 300, 17);
            s.parse("index/S,pindex/S,detector/B,sector/B,layer/B,c_x/F,c_y/F,c_z/F,c_ux/F,c_uy/F,c_uz/F,status/S");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int layer_;
        const int c_x_;
        const int c_y_;
        const int c_z_;
        const int c_ux_;
        const int c_uy_;
        const int c_uz_;
        const int status_;
    };

    /// Trajectory information for Particles bank
    class Traj {
    public:
        explicit Traj(hipo::bank& b) : b_(b),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            index_(b.getSchema().getEntryOrder("index")),
            detector_(b.getSchema().getEntryOrder("detector")),
            layer_(b.getSchema().getEntryOrder("layer")),
            x_(b.getSchema().getEntryOrder("x")),
            y_(b.getSchema().getEntryOrder("y")),
            z_(b.getSchema().getEntryOrder("z")),
            cx_(b.getSchema().getEntryOrder("cx")),
            cy_(b.getSchema().getEntryOrder("cy")),
            cz_(b.getSchema().getEntryOrder("cz")),
            path_(b.getSchema().getEntryOrder("path")),
            edge_(b.getSchema().getEntryOrder("edge"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the particle in REC::Particle
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// index of the track in its tracking bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// DetectorID as defined in org.jlab.detector.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Layer ID as defined in org.jlab.detector.DetectorLayer
        int layer(int row) const noexcept { return b_.getInt(layer_, row); }
        /// x-position of the track at the detector surface (cm)
        float x(int row) const noexcept { return b_.getFloat(x_, row); }
        /// y-position of the track at the detector surface (cm)
        float y(int row) const noexcept { return b_.getFloat(y_, row); }
        /// z-position of the track at the detector surface (cm)
        float z(int row) const noexcept { return b_.getFloat(z_, row); }
        /// direction cosline of the track at the detector surface
        float cx(int row) const noexcept { return b_.getFloat(cx_, row); }
        /// direction cosline of the track at the detector surface
        float cy(int row) const noexcept { return b_.getFloat(cy_, row); }
        /// direction cosline of the track at the detector surface
        float cz(int row) const noexcept { return b_.getFloat(cz_, row); }
        /// path length from primary vertex to the detector surface (cm)
        float path(int row) const noexcept { return b_.getFloat(path_, row); }
        /// signed distance between track position and closest detector edge (cm)
        float edge(int row) const noexcept { return b_.getFloat(edge_, row); }

        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_layer(int row, int8_t val) { b_.putByte(layer_, row, val); }
        void set_x(int row, float val) { b_.putFloat(x_, row, val); }
        void set_y(int row, float val) { b_.putFloat(y_, row, val); }
        void set_z(int row, float val) { b_.putFloat(z_, row, val); }
        void set_cx(int row, float val) { b_.putFloat(cx_, row, val); }
        void set_cy(int row, float val) { b_.putFloat(cy_, row, val); }
        void set_cz(int row, float val) { b_.putFloat(cz_, row, val); }
        void set_path(int row, float val) { b_.putFloat(path_, row, val); }
        void set_edge(int row, float val) { b_.putFloat(edge_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::Traj", 300, 18);
            s.parse("pindex/S,index/S,detector/B,layer/B,x/F,y/F,z/F,cx/F,cy/F,cz/F,path/F,edge/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int pindex_;
        const int index_;
        const int detector_;
        const int layer_;
        const int x_;
        const int y_;
        const int z_;
        const int cx_;
        const int cy_;
        const int cz_;
        const int path_;
        const int edge_;
    };

    /// Track information for Particles bank
    class UTrack {
    public:
        explicit UTrack(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            status_(b.getSchema().getEntryOrder("status")),
            q_(b.getSchema().getEntryOrder("q")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            NDF_(b.getSchema().getEntryOrder("NDF")),
            hbindex_(b.getSchema().getEntryOrder("hbindex")),
            px_(b.getSchema().getEntryOrder("px")),
            py_(b.getSchema().getEntryOrder("py")),
            pz_(b.getSchema().getEntryOrder("pz")),
            vx_(b.getSchema().getEntryOrder("vx")),
            vy_(b.getSchema().getEntryOrder("vy")),
            vz_(b.getSchema().getEntryOrder("vz"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the track in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// status of the track
        int status(int row) const noexcept { return b_.getInt(status_, row); }
        /// charge of the track
        int q(int row) const noexcept { return b_.getInt(q_, row); }
        /// Chi2 (or quality) track fitting
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// number of degrees of freedom in track fitting
        int NDF(int row) const noexcept { return b_.getInt(NDF_, row); }
        /// row number of corresponding track in RECHB::Track
        int hbindex(int row) const noexcept { return b_.getInt(hbindex_, row); }
        /// x component of the momentum (GeV)
        float px(int row) const noexcept { return b_.getFloat(px_, row); }
        /// y component of the momentum (GeV)
        float py(int row) const noexcept { return b_.getFloat(py_, row); }
        /// z component of the momentum (GeV)
        float pz(int row) const noexcept { return b_.getFloat(pz_, row); }
        /// x component of the vertex (cm)
        float vx(int row) const noexcept { return b_.getFloat(vx_, row); }
        /// y component of the vertex (cm)
        float vy(int row) const noexcept { return b_.getFloat(vy_, row); }
        /// z component of the vertex (cm)
        float vz(int row) const noexcept { return b_.getFloat(vz_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }
        void set_q(int row, int8_t val) { b_.putByte(q_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_NDF(int row, int16_t val) { b_.putShort(NDF_, row, val); }
        void set_hbindex(int row, int16_t val) { b_.putShort(hbindex_, row, val); }
        void set_px(int row, float val) { b_.putFloat(px_, row, val); }
        void set_py(int row, float val) { b_.putFloat(py_, row, val); }
        void set_pz(int row, float val) { b_.putFloat(pz_, row, val); }
        void set_vx(int row, float val) { b_.putFloat(vx_, row, val); }
        void set_vy(int row, float val) { b_.putFloat(vy_, row, val); }
        void set_vz(int row, float val) { b_.putFloat(vz_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::UTrack", 300, 45);
            s.parse("index/S,pindex/S,detector/B,sector/B,status/S,q/B,chi2/F,NDF/S,hbindex/S,px/F,py/F,pz/F,vx/F,vy/F,vz/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int status_;
        const int q_;
        const int chi2_;
        const int NDF_;
        const int hbindex_;
        const int px_;
        const int py_;
        const int pz_;
        const int vx_;
        const int vy_;
        const int vz_;
    };

    /// Track information for Particles bank
    class FTrack {
    public:
        explicit FTrack(hipo::bank& b) : b_(b),
            index_(b.getSchema().getEntryOrder("index")),
            pindex_(b.getSchema().getEntryOrder("pindex")),
            detector_(b.getSchema().getEntryOrder("detector")),
            sector_(b.getSchema().getEntryOrder("sector")),
            status_(b.getSchema().getEntryOrder("status")),
            q_(b.getSchema().getEntryOrder("q")),
            chi2_(b.getSchema().getEntryOrder("chi2")),
            NDF_(b.getSchema().getEntryOrder("NDF")),
            hbindex_(b.getSchema().getEntryOrder("hbindex")),
            px_(b.getSchema().getEntryOrder("px")),
            py_(b.getSchema().getEntryOrder("py")),
            pz_(b.getSchema().getEntryOrder("pz")),
            vx_(b.getSchema().getEntryOrder("vx")),
            vy_(b.getSchema().getEntryOrder("vy")),
            vz_(b.getSchema().getEntryOrder("vz"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// index of the track in the specific detector bank
        int index(int row) const noexcept { return b_.getInt(index_, row); }
        /// row number in the particle bank hit is associated with
        int pindex(int row) const noexcept { return b_.getInt(pindex_, row); }
        /// Detector ID, as defined in org.jlab.detector.base.DetectorType
        int detector(int row) const noexcept { return b_.getInt(detector_, row); }
        /// Sector of the track
        int sector(int row) const noexcept { return b_.getInt(sector_, row); }
        /// status of the track
        int status(int row) const noexcept { return b_.getInt(status_, row); }
        /// charge of the track
        int q(int row) const noexcept { return b_.getInt(q_, row); }
        /// Chi2 (or quality) track fitting
        float chi2(int row) const noexcept { return b_.getFloat(chi2_, row); }
        /// number of degrees of freedom in track fitting
        int NDF(int row) const noexcept { return b_.getInt(NDF_, row); }
        /// row number of corresponding track in RECHB::Track
        int hbindex(int row) const noexcept { return b_.getInt(hbindex_, row); }
        /// x component of the momentum (GeV)
        float px(int row) const noexcept { return b_.getFloat(px_, row); }
        /// y component of the momentum (GeV)
        float py(int row) const noexcept { return b_.getFloat(py_, row); }
        /// z component of the momentum (GeV)
        float pz(int row) const noexcept { return b_.getFloat(pz_, row); }
        /// x component of the vertex (cm)
        float vx(int row) const noexcept { return b_.getFloat(vx_, row); }
        /// y component of the vertex (cm)
        float vy(int row) const noexcept { return b_.getFloat(vy_, row); }
        /// z component of the vertex (cm)
        float vz(int row) const noexcept { return b_.getFloat(vz_, row); }

        void set_index(int row, int16_t val) { b_.putShort(index_, row, val); }
        void set_pindex(int row, int16_t val) { b_.putShort(pindex_, row, val); }
        void set_detector(int row, int8_t val) { b_.putByte(detector_, row, val); }
        void set_sector(int row, int8_t val) { b_.putByte(sector_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }
        void set_q(int row, int8_t val) { b_.putByte(q_, row, val); }
        void set_chi2(int row, float val) { b_.putFloat(chi2_, row, val); }
        void set_NDF(int row, int16_t val) { b_.putShort(NDF_, row, val); }
        void set_hbindex(int row, int16_t val) { b_.putShort(hbindex_, row, val); }
        void set_px(int row, float val) { b_.putFloat(px_, row, val); }
        void set_py(int row, float val) { b_.putFloat(py_, row, val); }
        void set_pz(int row, float val) { b_.putFloat(pz_, row, val); }
        void set_vx(int row, float val) { b_.putFloat(vx_, row, val); }
        void set_vy(int row, float val) { b_.putFloat(vy_, row, val); }
        void set_vz(int row, float val) { b_.putFloat(vz_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECHB::FTrack", 300, 48);
            s.parse("index/S,pindex/S,detector/B,sector/B,status/S,q/B,chi2/F,NDF/S,hbindex/S,px/F,py/F,pz/F,vx/F,vy/F,vz/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int index_;
        const int pindex_;
        const int detector_;
        const int sector_;
        const int status_;
        const int q_;
        const int chi2_;
        const int NDF_;
        const int hbindex_;
        const int px_;
        const int py_;
        const int pz_;
        const int vx_;
        const int vy_;
        const int vz_;
    };

} // namespace rechb

namespace recft {

    /// Event Header Bank, shadowing REC::Event with FT-based start time
    class Event {
    public:
        explicit Event(hipo::bank& b) : b_(b),
            category_(b.getSchema().getEntryOrder("category")),
            startTime_(b.getSchema().getEntryOrder("startTime"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// Undefined
        long category(int row) const noexcept { return b_.getLong(category_, row); }
        /// Event Start Time (ns)
        float startTime(int row) const noexcept { return b_.getFloat(startTime_, row); }

        void set_category(int row, int64_t val) { b_.putLong(category_, row, val); }
        void set_startTime(int row, float val) { b_.putFloat(startTime_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECFT::Event", 300, 41);
            s.parse("category/L,startTime/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int category_;
        const int startTime_;
    };

    /// Reconstructed Particle Information, shadowing REC::Particle with FT-based start time
    class Particle {
    public:
        explicit Particle(hipo::bank& b) : b_(b),
            pid_(b.getSchema().getEntryOrder("pid")),
            vt_(b.getSchema().getEntryOrder("vt")),
            beta_(b.getSchema().getEntryOrder("beta")),
            chi2pid_(b.getSchema().getEntryOrder("chi2pid")),
            status_(b.getSchema().getEntryOrder("status")),
            px_(b.getSchema().getEntryOrder("px")),
            py_(b.getSchema().getEntryOrder("py")),
            pz_(b.getSchema().getEntryOrder("pz"))
        {}

        int getRows() const noexcept { return b_.getRows(); }
        hipo::bank& bank() noexcept { return b_; }
        const hipo::bank& bank() const noexcept { return b_; }

        /// particle id in LUND conventions
        int pid(int row) const noexcept { return b_.getInt(pid_, row); }
        /// RF and z corrected vertex time (ns)
        float vt(int row) const noexcept { return b_.getFloat(vt_, row); }
        /// particle beta measured by TOF
        float beta(int row) const noexcept { return b_.getFloat(beta_, row); }
        /// Chi2 of assigned PID
        float chi2pid(int row) const noexcept { return b_.getFloat(chi2pid_, row); }
        /// particle status (represents detector collection it passed)
        int status(int row) const noexcept { return b_.getInt(status_, row); }
        /// x component of the momentum (GeV)
        float px(int row) const noexcept { return b_.getFloat(px_, row); }
        /// y component of the momentum (GeV)
        float py(int row) const noexcept { return b_.getFloat(py_, row); }
        /// z component of the momentum (GeV)
        float pz(int row) const noexcept { return b_.getFloat(pz_, row); }

        void set_pid(int row, int32_t val) { b_.putInt(pid_, row, val); }
        void set_vt(int row, float val) { b_.putFloat(vt_, row, val); }
        void set_beta(int row, float val) { b_.putFloat(beta_, row, val); }
        void set_chi2pid(int row, float val) { b_.putFloat(chi2pid_, row, val); }
        void set_status(int row, int16_t val) { b_.putShort(status_, row, val); }
        void set_px(int row, float val) { b_.putFloat(px_, row, val); }
        void set_py(int row, float val) { b_.putFloat(py_, row, val); }
        void set_pz(int row, float val) { b_.putFloat(pz_, row, val); }

        hipo::bank::rowlist::list_t const& getRowList() const { return b_.getRowList(); }
        hipo::bank::rowlist& getMutableRowList() { return b_.getMutableRowList(); }

        static hipo::schema schema() {
            hipo::schema s("RECFT::Particle", 300, 42);
            s.parse("pid/I,vt/F,beta/F,chi2pid/F,status/S,px/F,py/F,pz/F");
            return s;
        }

    private:
        hipo::bank& b_;
        const int pid_;
        const int vt_;
        const int beta_;
        const int chi2pid_;
        const int status_;
        const int px_;
        const int py_;
        const int pz_;
    };

} // namespace recft

} // namespace clas12
