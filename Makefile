PYTHON_FILES = tests/python/*.py fast_minh/*.py

build: fast_minh/* tests/cpp/*
	mkdir -p build
	cmake -B build/ -S fast_minh/ -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/ --config Debug

test:
	# run python tests
	pytest tests/python
	# run c++ tests
	cd build/tests/ && ctest -V

style:
	black -S $(PYTHON_FILES)
	isort $(PYTHON_FILES)
	ruff check $(PYTHON_FILES)
	clang-format -i fast_minh/*.cpp fast_minh/*.h
	clang-format -i tests/cpp/*.cpp
