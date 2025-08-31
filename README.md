# 🗂️ FUSE FileSystem

<div align="center">
  <img src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white" />
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white" />
  <img src="https://img.shields.io/badge/Docker-2496ED?style=for-the-badge&logo=docker&logoColor=white" />
  <img src="https://img.shields.io/badge/FUSE-FF6B35?style=for-the-badge&logo=linux&logoColor=white" />
  <img src="https://img.shields.io/badge/Operating_Systems-2E8B57?style=for-the-badge&logo=linux&logoColor=white" />
</div>
<div align="center">
  <img src="https://user-images.githubusercontent.com/73097560/115834477-dbab4500-a447-11eb-908a-139a6edaec5c.gif">
</div>


> **📚 Academic Project Notice**  
> This project was developed during our **third year** of the Computer Engineering program at the University of Buenos Aires. _This repository **does not** reflect our current programming level or professional skills. It is kept here as an academic and knowledge record._

---

## 🤖 About

**fisopfs** is a custom **FUSE-based userspace filesystem** developed for the **Operating Systems (7508)** course at **FIUBA (Méndez–Fresia)**. This filesystem implementation mounts a test directory and persists all filesystem state to a single on-disk file, enabling easy local development and Docker experimentation.

### 🎯 Key Features
- **Userspace Implementation** using FUSE (Filesystem in Userspace)
- **Persistent Storage** to single on-disk file
- **Docker support**
- **Hybrid Architecture** combining Python flexibility with C performance

### 🔧 Tech Stack
- **Primary**: Python for main filesystem logic
- **Supplemental**: C components for performance-critical operations
- **Build System**: Makefile-driven compilation and execution
- **Environment**: Linux with FUSE support required

---

## 📋 Prerequisites

- **Linux Environment** with FUSE available
- **Python** and **C** development tools
- **Make** build system
- **Docker** (optional, for containerized testing)

---

## 🛠️ Quick Start

### Compilation
```bash
$ make
```

### Setup Test Environment
Create your test directory:
```bash
$ mkdir prueba
```

### Launch FUSE Server
Start the filesystem from the compilation directory:
```bash
$ ./fisopfs prueba/
```

**Custom persistence file** (optional):
```bash
$ ./fisopfs prueba/ --filedisk custom_disk.fisopfs
```
> Default persistence file: `persistence_file.fisopfs`

### Verify Mount
Confirm the filesystem is properly mounted:
```bash
$ mount | grep fisopfs
```

### Test the Filesystem
Open a new terminal and interact with your mounted filesystem:
```bash
$ cd prueba
$ ls -al
# Create files, directories, test filesystem operations
```

### Cleanup
Unmount when finished:
```bash
$ sudo umount prueba
```

---

## 🐳 Docker Development

The project includes three Docker targets for easy development:

| Command | Description |
|---------|-------------|
| `docker-build` | Creates Ubuntu 20.04 image with FUSE dependencies |
| `docker-run` | Launches container with bash shell |
| `docker-attach` | Attaches to existing container for testing |

### Docker Workflow
```bash
# Build the development environment
$ make docker-build

# Run container and compile
$ make docker-run
# Inside container:
$ make
$ ./fisopfs -f ./prueba

# In another terminal, attach for testing
$ make docker-attach
# Navigate to prueba directory and test filesystem
```

---

### 📖 Theoretical Component
Theoretical answers and documentation are available in the `fisopfs.md` file included in this repository.  

---

## 🔧 Development Tools

### Code Formatting
Apply consistent code formatting:
```bash
$ make format
```

Commit formatting changes:
```bash
$ git add .
$ git commit -m "format: apply code formatter"
```

---

<div align="center">
  <img src="https://user-images.githubusercontent.com/73097560/115834477-dbab4500-a447-11eb-908a-139a6edaec5c.gif">
  
  **Built with 🐍 Python &⚡C at Universidad de Buenos Aires**
</div>
