#!/bin/bash

echo "=== Initializing Git Repository for Aerial Aces ==="
echo ""

# Initialize git repository
git init
echo "✓ Git repository initialized"

# Add all files
git add .
echo "✓ Files staged for commit"

# Create initial commit
git commit -m "Initial commit: Aerial Aces Flight Simulator

- Complete project structure with 8 directories
- Working C++ demo with flight controls and camera system
- CMake build configuration for cross-platform support
- Comprehensive documentation (README, QUICKSTART, BUILD, STRUCTURE)
- Asset management system and placeholder guides
- 16 files ready for development

Generated with Claude Code
https://claude.com/claude-code"

echo "✓ Initial commit created"
echo ""
echo "=== Repository Ready ==="
echo ""
echo "Next steps:"
echo "1. Create GitHub repository:"
echo "   https://github.com/new"
echo ""
echo "2. Add remote and push:"
echo "   git remote add origin https://github.com/YOUR_USERNAME/AerialAces-FlightSim.git"
echo "   git branch -M main"
echo "   git push -u origin main"
echo ""
echo "3. Start development:"
echo "   mkdir build && cd build"
echo "   cmake .. && make"
echo "   ./bin/AerialAces"
echo ""
