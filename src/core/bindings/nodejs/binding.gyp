{
  "targets": [
    {
      "target_name": "corebase_nodejs",
      "sources": [
        "src/native/corebase_addon.cpp",
        "src/native/error_handler_wrapper.cpp",
        "src/native/config_manager_wrapper.cpp",
        "src/native/system_monitor_wrapper.cpp",
        "src/native/network_manager_wrapper.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../../../..",
        "../../..",
        "../../../CoreClass",
        "../../../Monitoring"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.15"
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1
        }
      },
      "conditions": [
        [
          "OS=='win'",
          {
            "libraries": [
              "-lws2_32",
              "-lpsapi",
              "-lpdh"
            ],
            "defines": [
              "PLATFORM_WINDOWS",
              "WIN32_LEAN_AND_MEAN",
              "NOMINMAX"
            ]
          }
        ],
        [
          "OS=='mac'",
          {
            "libraries": [
              "-framework CoreFoundation",
              "-framework CoreServices",
              "-framework SystemConfiguration",
              "-framework IOKit"
            ],
            "defines": [
              "PLATFORM_MACOS"
            ]
          }
        ],
        [
          "OS=='linux'",
          {
            "libraries": [
              "-lpthread",
              "-ldl"
            ],
            "defines": [
              "PLATFORM_LINUX"
            ],
            "cflags": [
              "-fPIC"
            ]
          }
        ]
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS",
        "NODE_ADDON_API_DISABLE_DEPRECATED"
      ]
    }
  ]
}