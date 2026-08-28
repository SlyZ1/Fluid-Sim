CUDA_PATH ?= $(CUDA_PATH)

MSVC_BIN = C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64

CXX  = "$(MSVC_BIN)/cl.exe"
LINK = "$(MSVC_BIN)/link.exe"
LIBTOOL  = "$(MSVC_BIN)/lib.exe"
NVCC = "$(CUDA_PATH)/bin/nvcc.exe"

# flags de base, communs
CXXFLAGS_BASE = /nologo /Zi /std:c++17 /EHsc /W4 /MD /openmp
LIBS_BASE     = /LIBPATH:lib glfw3dll.lib nfd.lib ole32.lib uuid.lib dwmapi.lib

# ajouts spécifiques debug
ifeq ($(DEBUG),1)
    CXXFLAGS = $(CXXFLAGS_BASE) /fsanitize=address
    LIBS     = $(LIBS_BASE) /DEBUG
else
    CXXFLAGS = $(CXXFLAGS_BASE)
    LIBS     = $(LIBS_BASE)
endif

CXXFLAGS_SRC = $(CXXFLAGS) /showIncludes

NVCCFLAGS = -std=c++17 -ccbin "$(MSVC_BIN)/cl.exe" -g -Xcompiler /MD -arch=sm_89

INCLUDES = -Iinclude \
           -Iinclude/imgui \
           -Iinclude/nativefiledialog \

rwildcard = $(foreach d, $(wildcard $(1:=/*)), $(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC     = $(call rwildcard, src, *.cpp) $(call rwildcard, src, *.c)

OBJ    = $(patsubst src/%.cpp, build/src/%.obj, \
         $(patsubst src/%.c,   build/src/%.obj, $(SRC)))

DEP = $(OBJ:.obj=.d)

IMGUI_SRC   	= $(wildcard include/imgui/*.cpp)
IMGUI_OBJ   	= $(patsubst include/imgui/%.cpp,         	build/imgui/%.obj,   	$(IMGUI_SRC))

THIRD_PARTY_LIB = build/thirdparty.lib
TARGET          = myprogram.exe

all: $(TARGET)

$(TARGET): $(OBJ) $(THIRD_PARTY_LIB)
	@cmd /c "echo LIB=%LIB%"
	$(LINK) /nologo $(OBJ) $(THIRD_PARTY_LIB) /OUT:$@ $(LIBS)

build/src/%.obj: src/%.cpp
	if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS_SRC) $(INCLUDES) /c $< /Fo:$@ > "$(subst /,\,$@).tmp"
	powershell -NoProfile -ExecutionPolicy Bypass -File tools\gen_deps.ps1 -TmpFile "$(subst /,\,$@).tmp" -ObjFile "$(subst /,\,$@)" -SrcFile "$(subst /,\,$<)" -DepFile "$(subst /,\,$(@:.obj=.d))"

build/src/%.obj: src/%.c
	@if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
	$(CXX) $(CXXFLAGS_SRC) $(INCLUDES) /c $< /Fo:$@ > "$(subst /,\,$@).tmp"
	powershell -NoProfile -ExecutionPolicy Bypass -File tools\gen_deps.ps1 -TmpFile "$(subst /,\,$@).tmp" -ObjFile "$(subst /,\,$@)" -SrcFile "$(subst /,\,$<)" -DepFile "$(subst /,\,$(@:.obj=.d))"

$(THIRD_PARTY_LIB): $(IMGUI_OBJ)
	$(LIBTOOL) /nologo /OUT:$@ $^

build/imgui/%.obj: include/imgui/%.cpp | build/imgui
	$(CXX) $(CXXFLAGS) $(INCLUDES) /c $< /Fo:$@

build/imgui:
	if not exist "$(subst /,\,$@)" mkdir "$(subst /,\,$@)"

-include $(DEP)

clean:
	rmdir /S /Q build
	del /Q $(TARGET)

re: clean all

run: all
	$(TARGET) $(ARGS)

debug: clean
	$(MAKE) DEBUG=1 all
	$(TARGET) $(ARGS)