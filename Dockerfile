FROM ubuntu:22.04

# Set non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    python3 \
    python3-pip \
    python3-dev \
    libopenmpi-dev \
    texlive-latex-base \
    texlive-fonts-recommended \
    texlive-latex-extra \
    cm-super \
    dvipng \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install --no-cache-dir \
    pandas \
    numpy \
    matplotlib

# Set up directory structure
WORKDIR /app
RUN mkdir -p /app/exec_orders \
    /app/input_files \
    /app/output_files \
    /app/params_files \
    /app/ITL_tables \
    /app/traces \
    /app/Grid_VN2D \
    /app/Torus_3D \
    /app/Ring_1D \
    /app/Grid_VN3D

# Copy source files
COPY *.cpp *.h *.tpp Makefile /app/
COPY Grid_VN2D/*.cpp Grid_VN2D/*.h /app/Grid_VN2D/
COPY Torus_3D/*.cpp Torus_3D/*.h /app/Torus_3D/
COPY Ring_1D/*.cpp Ring_1D/*.h /app/Ring_1D/
COPY Grid_VN3D/*.cpp Grid_VN3D/*.h /app/Grid_VN3D/

# Copy Python scripts
COPY *.py /app/

# Compile the code
#RUN make

# Set up matplotlib to use a non-interactive backend
ENV MPLBACKEND=Agg

# Entry point (can be overridden)
ENTRYPOINT ["/bin/bash"]
