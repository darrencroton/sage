# SAGE Documentation Quick Reference

This document serves as a central index for all SAGE documentation.

## Getting Started

- **[README.md](../README.md)** - Main project overview, installation, and quick start guide
- **[CLAUDE.md](../CLAUDE.md)** - Development guide for Claude Code integration

## Architecture and Development

- **[directory-structure.md](directory-structure.md)** - Detailed explanation of the reorganized source code structure
- **[yaml-configuration-guide.md](yaml-configuration-guide.md)** - Complete YAML configuration documentation
- **[doc_standards.md](doc_standards.md)** - Documentation standards and templates
- **[error_handling_guidelines.md](error_handling_guidelines.md)** - Error handling best practices

## Project Planning and Progress

- **[log/phase.md](../log/phase.md)** - Current project phase status and objectives
- **[log/sage-master-plan.md](../log/sage-master-plan.md)** - Complete architectural transformation roadmap
- **[log/sage-architecture-vision.md](../log/sage-architecture-vision.md)** - Long-term architectural vision and principles
- **[log/architecture.md](../log/architecture.md)** - Current codebase architecture snapshot
- **[log/progress.md](../log/progress.md)** - Development progress tracking
- **[log/decisions.md](../log/decisions.md)** - Design decisions and rationale

## Source Code Organization

With the completion of Tasks 1.2 and 1.3, SAGE now follows a modern, modular structure with centralized memory management:

```
src/
├── core/           # Core infrastructure and coordination
├── physics/        # Physical process implementations
├── io/            # Input/output operations
├── utils/         # Utility functions and system management
└── scripts/       # Build and utility scripts
```

See [directory-structure.md](directory-structure.md) for complete details.

## Build and Development Tools

- **CMake build system** - Modern, cross-platform build configuration
- **src/scripts/beautify.sh** - Code formatting (C and Python)
- **src/scripts/first_run.sh** - Automated environment setup
- **output/sage-plot/** - Integrated plotting and visualization system

## Quick Commands

```bash
# Setup new environment
./src/scripts/first_run.sh

# Build SAGE
mkdir build && cd build && cmake .. && make -j$(nproc) && cd ..

# Run simulation
./sage input/millennium.yaml

# Generate plots
source sage_venv/bin/activate
python ./output/sage-plot/sage-plot.py --param-file=./input/millennium.yaml
deactivate

# Format code
./src/scripts/beautify.sh
```

## Development Guidelines

- Follow documentation standards in [doc_standards.md](doc_standards.md)
- Use error handling patterns from [error_handling_guidelines.md](error_handling_guidelines.md)
- Always check exit codes when running SAGE
- Update progress in [log/progress.md](../log/progress.md) for significant changes
- Archive outdated files to `scrap/` rather than deleting