#!/usr/bin/env python3
"""
Example Python configuration for electron analysis

This demonstrates CMSSW-style Python configuration for the CLAS12 framework.
Run with: python electron_analysis.py
"""

import pyclas12 as c12

# Create application
app = c12.Application("ElectronAnalysis")

# Configure services
hist_svc = c12.HistogramService("HistogramService")
hist_svc.set_output_file("electron_output.root")

cond_svc = c12.ConditionsService("ConditionsService")
cond_svc.set_run_number(5032)

# Add services to application
svc_mgr = app.service_manager()
svc_mgr.add_service(hist_svc)
svc_mgr.add_service(cond_svc)

# Configure input files
app.set_input_files([
    "data/run_5032.hipo",
    "data/run_5033.hipo"
])

# Processing options
app.set_max_events(100000)  # Process 100k events (-1 for all)
app.set_num_threads(4)       # Use 4 threads

# Run the analysis
if __name__ == "__main__":
    print("Starting CLAS12 Electron Analysis")
    print("=" * 60)

    # Initialize
    status = app.initialize()
    if not status.is_success():
        print("ERROR: Initialization failed")
        exit(1)

    # Run event loop
    status = app.run()
    if not status.is_success():
        print("ERROR: Event processing failed")
        exit(1)

    # Finalize
    status = app.finalize()
    if not status.is_success():
        print("WARNING: Finalization had issues")

    print("\nAnalysis complete!")
