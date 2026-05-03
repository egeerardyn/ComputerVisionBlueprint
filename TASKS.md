
# Milestones

## Short-term milestone (high impact)

- [x] create a justfile with the most common commands used [P0]
  - [x]  make sure this file can be used with either Linux or Windows
  - [x]  update the markdown documentation files to use the just file instead of calling the ps1/sh files directly
- [x] fix application crashes, it seems to happen when an image does not have the right datatype (e.g. putting a color image to a morphological operator) [P0]
- [ ] the light/dark mode switch does not properly work: light mode appears dark as well. Fix this [P0]
- [ ] add a test suite to test the available functionality [P0]
- [ ] improve error handling in nodes (clear error states/messages instead of crashing) [P0]
- [ ] add explicit datatype conversion nodes (depth/channel conversion with saturation and scaling options) [P0]
- [ ] add graph validation before execution (port compatibility, missing inputs, cycle detection) [P0]
- [ ] add CI pipeline to build and run tests on Windows and Linux [P0]

- [ ] implement OpenCV circle detection (via Hough transform) [P1]
- [ ] add Sobel/Scharr edge and gradient nodes [P1]
- [ ] add image histogram and histogram equalization/CLAHE nodes [P1]
- [ ] add connected components and contour analysis nodes (labeling, area, perimeter, moments) [P1]
- [ ] add template matching node (with selectable matching method) [P1]
- [ ] add video file stream input node [P1]

- [ ] add nodes to convert between images that use indexed and non-indexed colors [P1]
- [ ] add a node to save an image to file in different common file formats (e.g. TIFF, PNG, CSV, BMP, OpenEXR, ...) [P1]
- [ ] add an inspector dock, that shows the currently selected port's contents (e.g. show the image if it's an image) [P1]
- [ ] add undo/redo support for graph and node parameter edits [P1]
- [ ] add autosave and crash recovery for the current graph [P1]

- [ ] add a simple matrix editor to edit small image kernels (e.g. 3x3 up to 21x21) [P2]
- [ ] add a node to load an image kernel from a CSV file [P2]
- [ ] remove the built-in matrix editor on Filter2D and instead take the kernel as an input [P2]
- [ ] add ROI/cropping and mask editing tools/nodes [P2]
- [ ] add graph presets/examples for common workflows (denoise, thresholding, edge detection) [P2]

## Long-term milestone (strategic)

- [ ] add support for cv::Mat of complex, make this in a new division to allow for the following operations: [P2]
  - [ ] complex <-> amplitude and phase
  - [ ] complex <-> real and imaginary
  - [ ] FFT and IFFT in 2 dimensions
  - [ ] fftshift and iffshift
  - [ ] filtering in the frequency domain
  - [ ] a node to pad the image to a given size

- [ ] export the nodes to code [P2]
  - [ ] generate code in C++
  - [ ] generate code in Python
  - [ ] add another panel to show the code, including code highlighting

- [ ] add a node that is just a Note, i.e. contains multiple lines of text. In code generation it will generate comments [P2]
- [ ] allow to call an external script or program to take input parameters and/or an image and return results and/or an image [P2]
  - [ ] ideally such a script would be written in Python
- [ ] add batch processing mode to execute a graph on all images in a folder [P2]
- [ ] add performance profiling overlay (node execution time and memory usage) [P2]
- [ ] add optional OpenCV CUDA backend support for compatible nodes [P3]
- [ ] add a plugin API for third-party/custom nodes [P3]
