/**
 * @file test_batch_synthesis.cpp
 * @brief Round-trip tests for SWIF2 per-job TOML synthesis and command rendering.
 *
 * These exercise the pure-string transforms in `framework::BatchSystem`
 * without touching the filesystem-bound CLI driver in `tools/batch.cpp`.
 */

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <toml++/toml.hpp>

#include <clas12/framework/core/BatchSystem.hpp>

namespace fs = std::filesystem;
using framework::BatchSystem;

namespace {

/// Write a temp analysis TOML matching the schema documented in
/// AnalysisRunner.hpp, with an [ElectronAnalysis] section that the
/// synthesizer must preserve.
auto write_fixture_toml() -> std::string {
    const auto tmp = fs::temp_directory_path() /
                     fmt::format("batch_synthesis_fixture_{}.toml", ::getpid());
    std::ofstream out(tmp);
    out << R"([run]
input       = "/will/be/overridden.hipo"
output      = "ignored.root"
num_threads = 1
percentage  = 100.0

[analysis]
target      = "LD2"
polarity    = "Outbending"
beam_energy = 10.53

[ElectronAnalysis]
enable_chi2pid_cut = true
enable_vz_cut      = false
)";
    return tmp.string();
}

}  // namespace

// =============================================================================
// synthesize_job_toml
// =============================================================================

TEST(BatchSynthesis, OverridesRunInputAndOutputAndThreads) {
    const auto fixture = write_fixture_toml();

    const std::vector<std::string> locals = {
        "input_0.hipo", "input_1.hipo", "input_2.hipo"
    };
    auto text = BatchSystem::synthesize_job_toml(fixture, locals, "result.root", 8);

    auto parsed = toml::parse(text);
    ASSERT_TRUE(parsed["run"].is_table()) << "synthesized TOML missing [run]";

    auto* run = parsed["run"].as_table();
    auto  input_arr = (*run)["input"].as_array();
    ASSERT_NE(input_arr, nullptr) << "[run].input is not an array";
    ASSERT_EQ(input_arr->size(), 3u);
    EXPECT_EQ(*(*input_arr)[0].value<std::string>(), "input_0.hipo");
    EXPECT_EQ(*(*input_arr)[1].value<std::string>(), "input_1.hipo");
    EXPECT_EQ(*(*input_arr)[2].value<std::string>(), "input_2.hipo");

    EXPECT_EQ(*(*run)["output"].value<std::string>(), "result.root");
    EXPECT_EQ(*(*run)["num_threads"].value<int64_t>(), 8);

    fs::remove(fixture);
}

TEST(BatchSynthesis, PreservesAnalysisAndAlgorithmSections) {
    const auto fixture = write_fixture_toml();
    auto text = BatchSystem::synthesize_job_toml(
        fixture, {"input_0.hipo"}, "result.root", 4);

    auto parsed = toml::parse(text);
    auto* analysis = parsed["analysis"].as_table();
    ASSERT_NE(analysis, nullptr);
    EXPECT_EQ(*(*analysis)["target"].value<std::string>(),   "LD2");
    EXPECT_EQ(*(*analysis)["polarity"].value<std::string>(), "Outbending");
    EXPECT_DOUBLE_EQ(*(*analysis)["beam_energy"].value<double>(), 10.53);

    auto* alg = parsed["ElectronAnalysis"].as_table();
    ASSERT_NE(alg, nullptr) << "per-algorithm section was dropped";
    EXPECT_TRUE(*(*alg)["enable_chi2pid_cut"].value<bool>());
    EXPECT_FALSE(*(*alg)["enable_vz_cut"].value<bool>());

    fs::remove(fixture);
}

// =============================================================================
// swif2_add_job_command
// =============================================================================

TEST(BatchSynthesis, Swif2AddJobIncludesInputsOutputsAndShell) {
    BatchSystem::Swif2JobConfig cfg;
    cfg.workflow   = "test_wf";
    cfg.account    = "clas12";
    cfg.partition  = "production";
    cfg.cores      = 4;
    cfg.ram_gb     = 8;
    cfg.disk_gb    = 20;
    cfg.time       = "06:00:00";  // converted to seconds for SWIF2
    cfg.executable          = "/work/clas12/me/builddir/electron_analysis";
    cfg.modules             = {"gcc/13.2.0", "root/6.36.04"};
    cfg.env                 = {{"ROOT_HIST", "0"}};
    cfg.tags                = {{"stream", "electron"}};
    cfg.wrapper_script_path = "/abs/path/test_wf_wrapper.sh";

    BatchSystem::JobSpec job;
    job.job_name      = "test_wf_job_0";
    job.job_toml_path = "/abs/path/test_wf_job_0.toml";
    job.inputs        = {
        {"input_0.hipo", "/mss/clas12/foo/run_0.hipo"},
        {"input_1.hipo", "/mss/clas12/foo/run_1.hipo"},
    };
    job.outputs = {
        {"result.root", "/volatile/clas12/me/test_wf/job_0/result.root"},
        {"log.txt",     "/volatile/clas12/me/test_wf/logs/test_wf_job_0.log"},
    };

    const auto cmd = BatchSystem::swif2_add_job_command(cfg, job);

    // Frame
    EXPECT_NE(cmd.find("swif2 add-job"), std::string::npos);
    EXPECT_NE(cmd.find("-workflow 'test_wf'"), std::string::npos);
    EXPECT_NE(cmd.find("-name 'test_wf_job_0'"), std::string::npos);
    EXPECT_NE(cmd.find("-account 'clas12'"), std::string::npos);
    EXPECT_NE(cmd.find("-cores 4"), std::string::npos);
    EXPECT_NE(cmd.find("-ram 8G"), std::string::npos);
    EXPECT_NE(cmd.find("-disk 20G"), std::string::npos);
    // 06:00:00 must be converted into a SWIF2-acceptable form (seconds).
    EXPECT_NE(cmd.find("-time '21600s'"), std::string::npos);

    // Tags
    EXPECT_NE(cmd.find("-tag 'stream' 'electron'"), std::string::npos);

    // Inputs (data + the per-job TOML staged as job.toml)
    EXPECT_NE(cmd.find("-input 'input_0.hipo' '/mss/clas12/foo/run_0.hipo'"), std::string::npos);
    EXPECT_NE(cmd.find("-input 'input_1.hipo' '/mss/clas12/foo/run_1.hipo'"), std::string::npos);
    EXPECT_NE(cmd.find("-input job.toml '/abs/path/test_wf_job_0.toml'"), std::string::npos);

    // Outputs
    EXPECT_NE(cmd.find("-output 'result.root' '/volatile/clas12/me/test_wf/job_0/result.root'"), std::string::npos);
    EXPECT_NE(cmd.find("-output 'log.txt' '/volatile/clas12/me/test_wf/logs/test_wf_job_0.log'"), std::string::npos);

    // Wrapper staging + command. Module loads / env / executable now live
    // in the wrapper script; the add-job line just runs `bash wrapper.sh`.
    EXPECT_NE(cmd.find("-input wrapper.sh '/abs/path/test_wf_wrapper.sh'"), std::string::npos);
    EXPECT_NE(cmd.find("bash wrapper.sh"), std::string::npos);
    EXPECT_EQ(cmd.find("-shell"),          std::string::npos);
}

TEST(BatchSynthesis, WrapperScriptCarriesModulesEnvAndExecutable) {
    BatchSystem::Swif2JobConfig cfg;
    cfg.workflow   = "wrap_demo";
    cfg.cores      = 4;
    cfg.executable = "/work/clas12/me/builddir/electron_analysis";
    cfg.modules    = {"gcc/13.2.0", "root/6.36.04"};
    cfg.env        = {{"ROOT_HIST", "0"}};

    const auto sh = BatchSystem::synthesize_wrapper_script(cfg);
    EXPECT_NE(sh.find("#!/usr/bin/env bash"),     std::string::npos);
    EXPECT_NE(sh.find("set -euo pipefail"),       std::string::npos);
    EXPECT_NE(sh.find("module load gcc/13.2.0"),  std::string::npos);
    EXPECT_NE(sh.find("module load root/6.36.04"), std::string::npos);
    EXPECT_NE(sh.find("export OMP_NUM_THREADS=${SWIF2_JOB_CORES:-4}"), std::string::npos);
    EXPECT_NE(sh.find("export ROOT_HIST=0"),      std::string::npos);
    EXPECT_NE(sh.find("/work/clas12/me/builddir/electron_analysis --config job.toml"),
              std::string::npos);
    EXPECT_NE(sh.find("tee log.txt"),             std::string::npos);
}

// =============================================================================
// format_swif2_time
// =============================================================================

TEST(BatchSynthesis, FormatSwif2TimeConvertsHHMMSSToSeconds) {
    EXPECT_EQ(BatchSystem::format_swif2_time("12:00:00"), "43200s");
    EXPECT_EQ(BatchSystem::format_swif2_time("06:00:00"), "21600s");
    EXPECT_EQ(BatchSystem::format_swif2_time("00:30:00"), "1800s");
    EXPECT_EQ(BatchSystem::format_swif2_time("01:02:03"), "3723s");
}

TEST(BatchSynthesis, FormatSwif2TimeAcceptsHHMM) {
    // HH:MM (no seconds) treated as HH:MM:00.
    EXPECT_EQ(BatchSystem::format_swif2_time("00:30"), "1800s");
}

TEST(BatchSynthesis, FormatSwif2TimePassesNativeFormsThrough) {
    // Already-SWIF2 forms must be returned unchanged.
    EXPECT_EQ(BatchSystem::format_swif2_time("12h"),    "12h");
    EXPECT_EQ(BatchSystem::format_swif2_time("720m"),   "720m");
    EXPECT_EQ(BatchSystem::format_swif2_time("43200s"), "43200s");
    EXPECT_EQ(BatchSystem::format_swif2_time("43200"),  "43200");
    EXPECT_EQ(BatchSystem::format_swif2_time(""),       "");
}

// =============================================================================
// split_input_files
// =============================================================================

TEST(BatchSynthesis, SplitInputFilesHandlesPartialChunks) {
    std::vector<std::string> in = {"a", "b", "c", "d", "e"};
    auto chunks = BatchSystem::split_input_files(in, 2);
    ASSERT_EQ(chunks.size(), 3u);
    EXPECT_EQ(chunks[0].size(), 2u);
    EXPECT_EQ(chunks[1].size(), 2u);
    EXPECT_EQ(chunks[2].size(), 1u);
    EXPECT_EQ(chunks[2][0], "e");
}

TEST(BatchSynthesis, SplitInputFilesPreservesOrderSinglePerJob) {
    std::vector<std::string> in = {"x", "y", "z"};
    auto chunks = BatchSystem::split_input_files(in, 1);
    ASSERT_EQ(chunks.size(), 3u);
    EXPECT_EQ(chunks[0][0], "x");
    EXPECT_EQ(chunks[1][0], "y");
    EXPECT_EQ(chunks[2][0], "z");
}

// =============================================================================
// synthesize_workflow_script
// =============================================================================

TEST(BatchSynthesis, WorkflowScriptFramesCreateAndRun) {
    BatchSystem::Swif2JobConfig cfg;
    cfg.workflow   = "wf_demo";
    cfg.site       = "jlab";
    cfg.cores      = 2;
    cfg.executable = "/bin/true";
    cfg.output_dir = "/volatile/me/wf_demo";
    cfg.log_dir    = "/volatile/me/wf_demo/logs";

    BatchSystem::JobSpec job;
    job.job_name      = "wf_demo_job_0";
    job.job_toml_path = "/tmp/wf_demo_job_0.toml";
    job.inputs        = {{"input_0.hipo", "/mss/x.hipo"}};
    job.outputs       = {{"result.root", "/volatile/me/wf_demo/wf_demo_job_0/result.root"}};

    auto script = BatchSystem::synthesize_workflow_script(cfg, {job});

    EXPECT_NE(script.find("#!/usr/bin/env bash"), std::string::npos);
    EXPECT_NE(script.find("set -euo pipefail"),    std::string::npos);
    EXPECT_NE(script.find("swif2 create -workflow 'wf_demo' -site 'jlab'"),
              std::string::npos);
    EXPECT_NE(script.find("swif2 add-job"),        std::string::npos);
    EXPECT_NE(script.find("swif2 run -workflow 'wf_demo'"), std::string::npos);
}

TEST(BatchSynthesis, WorkflowScriptCreatesOutputDirs) {
    BatchSystem::Swif2JobConfig cfg;
    cfg.workflow   = "wf_mkdir";
    cfg.site       = "jlab";
    cfg.cores      = 1;
    cfg.executable = "/bin/true";
    cfg.output_dir = "/volatile/me/wf_mkdir";
    cfg.log_dir    = "/volatile/me/wf_mkdir/logs";

    BatchSystem::JobSpec j0;
    j0.job_name      = "wf_mkdir_job_0";
    j0.job_toml_path = "/tmp/wf_mkdir_job_0.toml";
    j0.outputs       = {{"result.root", "/volatile/me/wf_mkdir/wf_mkdir_job_0/result.root"}};

    BatchSystem::JobSpec j1;
    j1.job_name      = "wf_mkdir_job_1";
    j1.job_toml_path = "/tmp/wf_mkdir_job_1.toml";
    j1.outputs       = {{"result.root", "/volatile/me/wf_mkdir/wf_mkdir_job_1/result.root"}};

    auto script = BatchSystem::synthesize_workflow_script(cfg, {j0, j1});

    // Top-level output and log dirs.
    EXPECT_NE(script.find("mkdir -p '/volatile/me/wf_mkdir'"),      std::string::npos);
    EXPECT_NE(script.find("mkdir -p '/volatile/me/wf_mkdir/logs'"), std::string::npos);
    // Per-job subdirectories so SWIF2 -output destinations resolve.
    EXPECT_NE(script.find("mkdir -p '/volatile/me/wf_mkdir/wf_mkdir_job_0'"), std::string::npos);
    EXPECT_NE(script.find("mkdir -p '/volatile/me/wf_mkdir/wf_mkdir_job_1'"), std::string::npos);
    // mkdir lines must come before swif2 create.
    auto pos_mkdir  = script.find("mkdir -p '/volatile/me/wf_mkdir'");
    auto pos_create = script.find("swif2 create");
    ASSERT_NE(pos_mkdir,  std::string::npos);
    ASSERT_NE(pos_create, std::string::npos);
    EXPECT_LT(pos_mkdir, pos_create);
}
