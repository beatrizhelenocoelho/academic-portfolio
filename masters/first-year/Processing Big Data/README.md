\# Processing Big Data



Project developed for the Processing Big Data course of the Master's Degree in

Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project Overview



The project developed a multimodal data-processing pipeline for analysing large collections

of television debates and newscasts.



The available datasets contained different types of information extracted from video,

audio and speech.



The objective was to combine these data sources in order to study people, topics and

content across multiple television recordings.



\## Data Modalities



The project worked with several forms of extracted information:



\### Visual Data



Visual information included:



\- Detected people

\- Detected faces

\- Facial landmarks

\- Body-pose keypoints

\- Emotion predictions

\- Screen presence



Processing steps were implemented to clean incorrect detections and associate visual

detections with the correct individuals.



\### Audio Data



Audio analysis included:



\- Speaker embeddings

\- Speaker identification

\- Speech rate

\- Pauses

\- Vocal intensity

\- Pitch variation



Speaker embeddings were used to help determine who was speaking and when.



\### Speech Data



Speech information contained:



\- Timestamps

\- Segment duration

\- Transcriptions

\- Text embeddings



Speech was divided into fixed time windows and analysed to determine dominant topics.



\### OCR Data



Text appearing on screen was analysed through OCR.



The processing included:



\- OCR quality analysis

\- Text cleaning

\- Lower-third analysis

\- Reference dictionaries

\- Identification of names and topics

\- Comparison between television channels



\## Multimodal Processing Pipeline



The project was organised into three main stages.



\### 1. Exploratory Data Analysis



Initial notebooks were used to understand:



\- OCR structure and quality

\- Political-content extraction

\- Differences between different data sources and channels



\### 2. Individual Newscast Analysis



Individual recordings were processed separately.



OCR information and speech information were analysed independently.



Speech was divided into 30-second windows and assigned dominant topics.



\### 3. Multimodal Fusion



The outputs from speech and OCR analysis were then combined.



This provided a richer representation of each news segment using both spoken content and

visual text.



The resulting data was used to compare:



\- Related topics across recordings

\- Similar news items

\- Temporal alignment between similar content

\- Debate topics and newscast topics



\## Candidate Profile Analysis



Visual, audio and speech features were also combined to create multimodal profiles based on

the available data.



These profiles integrated information such as:



\- Screen presence

\- Speaking activity

\- Visual behaviour

\- Audio characteristics

\- Spoken content



\## Implementation



Most of the analysis was implemented using Python and Jupyter notebooks.



The repository contains separate pipelines for:



\- Audio analysis

\- Speech analysis

\- OCR analysis

\- Visual-data cleaning

\- Identity association

\- Multimodal profile generation

\- Cross-source analysis



\## Technologies



\- Python

\- Jupyter Notebook

\- Pandas

\- NumPy

\- Data processing

\- OCR

\- Speech analysis

\- Audio embeddings

\- Multimodal data fusion

\- Computer vision data

\- Large-scale dataset analysis

