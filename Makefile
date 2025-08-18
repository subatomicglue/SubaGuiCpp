all:
	rm -rf ./build
	conan install . --output-folder=build --build=missing -s build_type=Release
	cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
	cmake --build build

run:
	./build/examples/standalone_app/standalone_app

debug: debug-help
	lldb ./build/examples/standalone_app/standalone_app

debug-help:
	@echo "========================================================================================"
	@echo "DEBUG HELP"
	@echo "========================================================================================"
	@echo "(lldb) breakpoint set --name main           # Set a breakpoint at the main function"
	@echo "(lldb) breakpoint set --file source.cpp --line 42  # Set a breakpoint at line 42 in source.cpp"
	@echo "(lldb) run"
	@echo "(lldb) print variableName  # Print the value of your variable"
	@echo "(lldb) step"
	@echo "(lldb) next"
	@echo "(lldb) continue"
	@echo "(lldb) bt  # Backtrace to show the stack"
	@echo "(lldb) exit"
	@echo "========================================================================================"

versions:
	conan search vulkan-loader  | grep -E 'vulkan-loader/[0-9]'
	conan search vulkan-headers | grep -E 'vulkan-headers/[0-9]'
	conan search moltenvk  | grep -E 'moltenvk/[0-9]'
