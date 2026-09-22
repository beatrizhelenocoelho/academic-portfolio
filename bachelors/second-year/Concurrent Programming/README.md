\# Parallel Image Processing



Project developed for the Concurrent Programming course of the Bachelor's Degree in Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project Objective



The objective of this project was to parallelise an image-processing application and evaluate the performance improvement obtained through concurrent execution.



The application applies a sequence of image-processing operations to a collection of JPEG images in order to generate an "old photo" effect.



The processing pipeline includes:



1\. Contrast adjustment

2\. Image smoothing

3\. Texture application

4\. Sepia transformation

5\. Output image generation



The project was implemented in C using POSIX threads.



\## Parallel Version A



The first implementation uses a configurable number of worker threads.



The list of input images is divided between the available threads.



Each thread receives a range of images and independently performs the complete processing sequence for those images.



For every assigned image, the thread:



1\. Loads the image.

2\. Applies the contrast transformation.

3\. Applies image smoothing.

4\. Applies a paper texture.

5\. Applies the sepia effect.

6\. Writes the processed image to the output directory.



The program also checks whether an output image has already been generated in order to avoid unnecessary processing.



Execution times are recorded for each thread, allowing the performance of different thread counts to be compared.



\## Parallel Version B



A second parallel implementation was developed to explore a different distribution of work between threads.



This version also uses POSIX threads and processes multiple images concurrently while collecting execution-time information.



The objective was to compare alternative parallelisation strategies and analyse their effect on execution time and workload distribution.



\## Pipeline Version



A pipeline-based implementation was also developed.



Instead of assigning an entire image-processing sequence to the same worker, the processing stages are organised as a pipeline.



Images move through successive processing stages, allowing different stages to operate concurrently on different images.



This architecture makes it possible to exploit parallelism not only between images but also between the individual processing operations.



\## Performance Evaluation



The implementations were tested using multiple image datasets and different numbers of threads.



Timing results were collected for configurations using:



\- 1 thread

\- 2 threads

\- 4 threads

\- 8 threads

\- 16 threads

\- 32 threads



These measurements were used to analyse:



\- Execution time

\- Parallel speedup

\- Scalability

\- Work distribution

\- Differences between the parallel and pipeline approaches



\## Implementation



The project uses:



\- POSIX Threads (`pthread`)

\- C

\- libgd

\- Dynamic memory allocation

\- File and directory management

\- Timing using `clock\_gettime`

\- Parallel workload distribution



\## Project Structure



```text

concurrent-programming/

├── README.md

├── parallel-version-a/

│   ├── old-photo-paralelo-A.c

│   ├── image-lib.c

│   ├── image-lib.h

│   ├── Makefile

│   └── timing-results/

├── parallel-version-b/

│   ├── old-photo-paralelo-B.c

│   └── Makefile

├── pipeline-version/

│   ├── old-photo-pipeline.c

│   ├── Makefile

│   └── timing-results/

└── reports/

