//! Build script for CoreBase Rust bindings
//!
//! This script configures the compilation and linking of the Rust bindings
//! with the C++ CoreBaseApplication library.

use std::env;
use std::path::PathBuf;

fn main() {
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=src/");
    println!("cargo:rerun-if-changed=../../"); // C++ source changes
    
    // Get the target OS
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    
    // Configure library paths based on the build environment
    configure_library_paths(&target_os, &target_arch);
    
    // Link required system libraries
    link_system_libraries(&target_os);
    
    // Configure C++ compilation if building from source
    if env::var("COREBASE_BUILD_FROM_SOURCE").is_ok() {
        build_cpp_library(&target_os, &target_arch);
    }
    
    // Generate bindings if requested
    if env::var("COREBASE_GENERATE_BINDINGS").is_ok() {
        generate_bindings();
    }
}

/// Configure library search paths
fn configure_library_paths(target_os: &str, target_arch: &str) {
    // Get the project root directory
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let project_root = PathBuf::from(&manifest_dir)
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .parent()
        .unwrap();
    
    // Add library search paths
    let lib_dir = project_root.join("lib");
    let build_dir = project_root.join("build");
    let target_dir = project_root.join("target");
    
    if lib_dir.exists() {
        println!("cargo:rustc-link-search=native={}", lib_dir.display());
    }
    
    if build_dir.exists() {
        println!("cargo:rustc-link-search=native={}", build_dir.display());
    }
    
    if target_dir.exists() {
        println!("cargo:rustc-link-search=native={}", target_dir.display());
    }
    
    // Platform-specific library paths
    match target_os {
        "windows" => {
            // Windows-specific paths
            println!("cargo:rustc-link-search=native=C:\\Program Files\\CoreBaseApplication\\lib");
            println!("cargo:rustc-link-search=native=C:\\Program Files (x86)\\CoreBaseApplication\\lib");
            
            // Visual Studio paths
            if let Ok(vs_path) = env::var("VCINSTALLDIR") {
                let vs_lib = PathBuf::from(vs_path).join("lib").join(target_arch);
                if vs_lib.exists() {
                    println!("cargo:rustc-link-search=native={}", vs_lib.display());
                }
            }
        },
        "linux" => {
            // Linux-specific paths
            println!("cargo:rustc-link-search=native=/usr/local/lib");
            println!("cargo:rustc-link-search=native=/usr/lib");
            println!("cargo:rustc-link-search=native=/opt/corebase/lib");
            
            // Architecture-specific paths
            match target_arch {
                "x86_64" => {
                    println!("cargo:rustc-link-search=native=/usr/lib/x86_64-linux-gnu");
                    println!("cargo:rustc-link-search=native=/usr/local/lib/x86_64-linux-gnu");
                },
                "aarch64" => {
                    println!("cargo:rustc-link-search=native=/usr/lib/aarch64-linux-gnu");
                    println!("cargo:rustc-link-search=native=/usr/local/lib/aarch64-linux-gnu");
                },
                _ => {}
            }
        },
        "macos" => {
            // macOS-specific paths
            println!("cargo:rustc-link-search=native=/usr/local/lib");
            println!("cargo:rustc-link-search=native=/opt/homebrew/lib");
            println!("cargo:rustc-link-search=native=/Applications/CoreBaseApplication.app/Contents/Frameworks");
        },
        _ => {}
    }
    
    // Try to find CoreBase library
    println!("cargo:rustc-link-lib=corebase");
    println!("cargo:rustc-link-lib=corebase_api");
}

/// Link required system libraries
fn link_system_libraries(target_os: &str) {
    match target_os {
        "windows" => {
            // Windows system libraries
            println!("cargo:rustc-link-lib=kernel32");
            println!("cargo:rustc-link-lib=user32");
            println!("cargo:rustc-link-lib=ws2_32");
            println!("cargo:rustc-link-lib=advapi32");
            println!("cargo:rustc-link-lib=shell32");
            println!("cargo:rustc-link-lib=ole32");
            println!("cargo:rustc-link-lib=oleaut32");
            println!("cargo:rustc-link-lib=uuid");
            println!("cargo:rustc-link-lib=winmm");
            println!("cargo:rustc-link-lib=psapi");
            println!("cargo:rustc-link-lib=pdh");
            
            // C++ runtime
            println!("cargo:rustc-link-lib=msvcrt");
        },
        "linux" => {
            // Linux system libraries
            println!("cargo:rustc-link-lib=pthread");
            println!("cargo:rustc-link-lib=dl");
            println!("cargo:rustc-link-lib=m");
            println!("cargo:rustc-link-lib=rt");
            
            // C++ standard library
            println!("cargo:rustc-link-lib=stdc++");
            
            // Optional libraries (check if available)
            if pkg_config::probe("openssl").is_ok() {
                println!("cargo:rustc-link-lib=ssl");
                println!("cargo:rustc-link-lib=crypto");
            }
        },
        "macos" => {
            // macOS system libraries
            println!("cargo:rustc-link-lib=framework=Foundation");
            println!("cargo:rustc-link-lib=framework=CoreFoundation");
            println!("cargo:rustc-link-lib=framework=SystemConfiguration");
            println!("cargo:rustc-link-lib=framework=Security");
            
            // C++ standard library
            println!("cargo:rustc-link-lib=c++");
            
            // System libraries
            println!("cargo:rustc-link-lib=pthread");
            println!("cargo:rustc-link-lib=dl");
            println!("cargo:rustc-link-lib=m");
        },
        _ => {
            // Generic Unix-like system
            println!("cargo:rustc-link-lib=pthread");
            println!("cargo:rustc-link-lib=dl");
            println!("cargo:rustc-link-lib=m");
            println!("cargo:rustc-link-lib=stdc++");
        }
    }
}

/// Build the C++ library from source
fn build_cpp_library(target_os: &str, target_arch: &str) {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let project_root = PathBuf::from(&manifest_dir)
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .parent()
        .unwrap();
    
    let src_dir = project_root.join("src").join("core");
    let out_dir = env::var("OUT_DIR").unwrap();
    let out_path = PathBuf::from(&out_dir);
    
    // Configure C++ compiler
    let mut build = cc::Build::new();
    
    // Add source files
    build
        .cpp(true)
        .std("c++17")
        .include(&src_dir)
        .include(src_dir.join("include"))
        .file(src_dir.join("CoreAPI.cpp"))
        .file(src_dir.join("SystemMonitor.cpp"))
        .file(src_dir.join("NetworkManager.cpp"))
        .file(src_dir.join("ConfigManager.cpp"))
        .file(src_dir.join("ErrorHandler.cpp"));
    
    // Add Java bindings if available
    let java_bindings = src_dir.join("bindings").join("java").join("JavaBindings.cpp");
    if java_bindings.exists() {
        build.file(&java_bindings);
        
        // Try to find JNI headers
        if let Ok(java_home) = env::var("JAVA_HOME") {
            let jni_include = PathBuf::from(&java_home).join("include");
            if jni_include.exists() {
                build.include(&jni_include);
                
                // Platform-specific JNI headers
                match target_os {
                    "windows" => {
                        build.include(jni_include.join("win32"));
                    },
                    "linux" => {
                        build.include(jni_include.join("linux"));
                    },
                    "macos" => {
                        build.include(jni_include.join("darwin"));
                    },
                    _ => {}
                }
            }
        }
    }
    
    // Platform-specific configuration
    match target_os {
        "windows" => {
            build
                .define("WIN32", None)
                .define("_WIN32", None)
                .define("_WINDOWS", None)
                .define("UNICODE", None)
                .define("_UNICODE", None)
                .flag("/EHsc") // Exception handling
                .flag("/MT");  // Static runtime
        },
        "linux" => {
            build
                .define("_GNU_SOURCE", None)
                .flag("-fPIC")
                .flag("-pthread");
        },
        "macos" => {
            build
                .define("_DARWIN_C_SOURCE", None)
                .flag("-fPIC")
                .flag("-pthread");
        },
        _ => {
            build.flag("-fPIC");
        }
    }
    
    // Optimization flags
    if env::var("PROFILE").unwrap() == "release" {
        match target_os {
            "windows" => {
                build.flag("/O2").flag("/DNDEBUG");
            },
            _ => {
                build.flag("-O3").flag("-DNDEBUG");
            }
        }
    } else {
        match target_os {
            "windows" => {
                build.flag("/Od").flag("/D_DEBUG");
            },
            _ => {
                build.flag("-O0").flag("-g").flag("-D_DEBUG");
            }
        }
    }
    
    // Compile the library
    build.compile("corebase_rust_bindings");
    
    println!("cargo:rustc-link-lib=static=corebase_rust_bindings");
    println!("cargo:rustc-link-search=native={}", out_path.display());
}

/// Generate C bindings using bindgen (optional)
fn generate_bindings() {
    // This would use bindgen to automatically generate Rust bindings
    // from C++ headers. For now, we're using manually written bindings.
    
    println!("cargo:warning=Automatic binding generation not implemented yet");
    println!("cargo:warning=Using manually written bindings instead");
    
    // Example of how bindgen would be used:
    /*
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate bindings");
    
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
    */
}