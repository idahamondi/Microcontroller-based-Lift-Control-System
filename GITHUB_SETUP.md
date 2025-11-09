# GitHub Setup Instructions

## Step 1: Create a GitHub Repository

1. Go to https://github.com and sign in
2. Click the "+" icon in the top right corner
3. Select "New repository"
4. Name it: `lift-control-system` (or your preferred name)
5. Choose Public or Private
6. **DO NOT** initialize with README, .gitignore, or license (we already have these)
7. Click "Create repository"

## Step 2: Push to GitHub

After creating the repository, GitHub will show you commands. Use these:

```bash
cd "d:\Desktop\Lift System"
git remote add origin https://github.com/YOUR_USERNAME/lift-control-system.git
git branch -M main
git push -u origin main
```

Replace `YOUR_USERNAME` with your GitHub username and `lift-control-system` with your repository name if different.

## Alternative: Using SSH

If you have SSH keys set up with GitHub:

```bash
git remote add origin git@github.com:YOUR_USERNAME/lift-control-system.git
git branch -M main
git push -u origin main
```

## Note

Your repository is ready with:
- ✅ `lift_control_system.ino` - Main Arduino code
- ✅ `README.md` - Project documentation
- ✅ `.gitignore` - Excludes PDFs and other unnecessary files

The PDF and Word document files are excluded by .gitignore and won't be pushed to GitHub.

