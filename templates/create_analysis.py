#!/usr/bin/env python3
"""
Scaffolding script for creating new CLAS12 analyses.

This script generates the boilerplate code for new analyses, including:
- Analysis executable (.cpp file)
- Custom algorithm header (.hpp file)
- Meson build configuration (meson.build)

Usage:
    python create_analysis.py --name my_analysis
    python create_analysis.py --name my_analysis --type electron --with-algorithm
    python create_analysis.py --name my_analysis --output ../analyses

Examples:
    # Create a basic analysis in analyses/my_analysis/
    python create_analysis.py --name my_analysis

    # Create an electron analysis with a custom algorithm
    python create_analysis.py --name electron_study --type electron --with-algorithm

    # Create in a custom directory
    python create_analysis.py --name test_analysis --output /path/to/output
"""

import argparse
import os
import sys
from pathlib import Path
from string import Template


def snake_to_pascal(name: str) -> str:
    """Convert snake_case to PascalCase."""
    return ''.join(word.capitalize() for word in name.split('_'))


def snake_to_title(name: str) -> str:
    """Convert snake_case to Title Case."""
    return ' '.join(word.capitalize() for word in name.split('_'))


def load_template(template_dir: Path, template_name: str) -> Template:
    """Load a template file."""
    template_path = template_dir / template_name
    if not template_path.exists():
        print(f"Error: Template not found: {template_path}", file=sys.stderr)
        sys.exit(1)
    with open(template_path) as f:
        return Template(f.read())


def create_analysis(
    name: str,
    analysis_type: str,
    output_dir: Path,
    with_algorithm: bool,
    namespace: str,
) -> None:
    """Create a new analysis from templates."""
    template_dir = Path(__file__).parent

    # Create output directory
    analysis_dir = output_dir / name
    analysis_dir.mkdir(parents=True, exist_ok=True)

    # Common template substitutions
    class_name = snake_to_pascal(name)
    alg_class_name = f"{class_name}Alg"
    subs = {
        "ANALYSIS_NAME": name,
        "CLASS_NAME": class_name,
        "ALG_CLASS_NAME": alg_class_name,
        "FILENAME": f"{name}.cpp",
        "DESCRIPTION": f"{snake_to_title(name)} analysis for CLAS12 data",
        "OUTPUT_FILE": f"{name}_output.root",
        "NAMESPACE": namespace,
    }

    # Generate analysis executable
    analysis_template = load_template(template_dir / "analysis", "my_analysis.cpp.template")
    analysis_content = analysis_template.safe_substitute(subs)

    # If creating with algorithm, add the include
    if with_algorithm:
        analysis_content = analysis_content.replace(
            '// #include "${CLASS_NAME}.hpp"',
            f'#include "{alg_class_name}.hpp"'
        )
        analysis_content = analysis_content.replace(
            '// app.add_algorithm(std::make_unique<${CLASS_NAME}>());',
            f'app.add_algorithm(std::make_unique<{namespace}::{alg_class_name}>());'
        )
        analysis_content = analysis_content.replace('${CLASS_NAME}', class_name)
        analysis_content = analysis_content.replace('${ALG_CLASS_NAME}', alg_class_name)

    with open(analysis_dir / f"{name}.cpp", "w") as f:
        f.write(analysis_content)
    print(f"Created: {analysis_dir / f'{name}.cpp'}")

    # Generate meson.build
    meson_template = load_template(template_dir / "analysis", "meson.build.template")
    meson_content = meson_template.safe_substitute(subs)
    with open(analysis_dir / "meson.build", "w") as f:
        f.write(meson_content)
    print(f"Created: {analysis_dir / 'meson.build'}")

    # Generate algorithm if requested
    if with_algorithm:
        alg_subs = subs.copy()
        alg_subs["CLASS_NAME"] = alg_class_name
        alg_subs["DESCRIPTION"] = f"Algorithm for {snake_to_title(name)} analysis"

        alg_template = load_template(template_dir / "algorithm", "MyAlgorithm.hpp.template")
        alg_content = alg_template.safe_substitute(alg_subs)
        with open(analysis_dir / f"{alg_class_name}.hpp", "w") as f:
            f.write(alg_content)
        print(f"Created: {analysis_dir / f'{alg_class_name}.hpp'}")

    # Print instructions
    print(f"\n✓ Analysis '{name}' created successfully in {analysis_dir}")
    print("\nNext steps:")
    print(f"  1. Edit {analysis_dir / f'{name}.cpp'} to customize your analysis")
    if with_algorithm:
        print(f"  2. Implement your physics logic in {analysis_dir / f'{alg_class_name}.hpp'}")
    print(f"  3. Add 'subdir('{name}')' to analyses/meson.build")
    print("  4. Rebuild with: meson compile -C build")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Create a new CLAS12 analysis from templates",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__.split("Usage:")[0],
    )

    parser.add_argument(
        "--name", "-n",
        required=True,
        help="Analysis name (snake_case, e.g., 'my_analysis', 'electron_study')",
    )
    parser.add_argument(
        "--type", "-t",
        default="general",
        choices=["general", "electron", "pion", "pi0", "lambda", "dis"],
        help="Analysis type (affects default algorithm structure)",
    )
    parser.add_argument(
        "--output", "-o",
        default=None,
        help="Output directory (default: ../analyses relative to this script)",
    )
    parser.add_argument(
        "--with-algorithm", "-a",
        action="store_true",
        help="Also generate a custom algorithm header file",
    )
    parser.add_argument(
        "--namespace",
        default="myanalysis",
        help="C++ namespace for generated code (default: myanalysis)",
    )

    args = parser.parse_args()

    # Determine output directory
    if args.output:
        output_dir = Path(args.output)
    else:
        # Default to analyses/ directory relative to project root
        script_dir = Path(__file__).parent
        output_dir = script_dir.parent / "analyses"

    # Validate name
    if not args.name.replace("_", "").isalnum():
        print("Error: Name must contain only letters, numbers, and underscores", file=sys.stderr)
        sys.exit(1)

    create_analysis(
        name=args.name,
        analysis_type=args.type,
        output_dir=output_dir,
        with_algorithm=args.with_algorithm,
        namespace=args.namespace,
    )


if __name__ == "__main__":
    main()
