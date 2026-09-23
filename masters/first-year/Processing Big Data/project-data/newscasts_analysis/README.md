# Big Data Project — Notebook Guide

This folder contains the notebooks used in the project.  
The notebooks are organized into three main parts:

1. **Part 1 — Exploratory Data Analysis (EDA)**
2. **Part 2 — Individual analysis of OCR and speech pickles**
3. **Part 3 — Multimodal / global analysis answering the final research questions**

The numbering of the notebooks was chosen to make the workflow easier to follow.

---

## Folder contents

```text
01_ocr_structure_quality_FIXED.ipynb
02_ocr_political_content_cleaned.ipynb
03_ocr_rtp_vs_tvi.ipynb
04_single_newscast_ocr_analysis_lowerthird_weighted_consolidated.ipynb
05_single_newscast_speech_30s_topic_analysis.ipynb
06_ocr_speech_fusion.ipynb
07_similar_news_across_channels_question8.ipynb
08_q7_batch_speech_30s_debates_vs_newscasts_v6_same_path_as_05.ipynb
```

---

# Part 1 — EDA notebooks

## `01_ocr_structure_quality_FIXED.ipynb`

This notebook belongs to the first exploratory analysis stage.

It studies the general structure and quality of the OCR data.  
The goal is to understand how the OCR information is organized, what fields are available, and whether the extracted text is usable for the following stages of the project.

This notebook is mainly exploratory and helps justify later processing choices.

---

## `02_ocr_political_content_cleaned.ipynb`

This notebook also belongs to the EDA stage.

It focuses on political content detected in the OCR data.  
The goal is to inspect and clean OCR text related to political/news content, so that the project can later work with more meaningful textual information.

This notebook is part of the initial understanding of the dataset.

---

## `03_ocr_rtp_vs_tvi.ipynb`

This notebook compares OCR information between the two TV channels, RTP and TVI.

It is still part of the EDA stage.  
The goal is to understand whether there are visible differences between the channels in terms of OCR content, structure, and extracted information.

This notebook helps motivate the later channel comparison used in the multimodal analysis.

---

# Part 2 — Individual pickle analysis

The next two notebooks are designed to analyse one newscast/pickle at a time.

They are useful for detailed inspection, debugging, and producing intermediate outputs that are later used by the fusion notebook.

---

## `04_single_newscast_ocr_analysis_lowerthird_weighted_consolidated.ipynb`

This notebook performs the individual OCR analysis for a single newscast pickle.

It analyses the OCR information of one selected newscast, with special focus on:

- lower-third text;
- text appearing on screen;
- OCR segments;
- consolidation of OCR information;
- weighting of lower-third information when relevant.

The output of this notebook represents the OCR-based interpretation of one newscast.

This notebook is expected to be run for the newscast/date that the user wants to analyse.

---

## `05_single_newscast_speech_30s_topic_analysis.ipynb`

This notebook performs the individual speech analysis for a single newscast pickle.

It analyses the transcription/speech information of one selected newscast.  
The speech is divided into fixed 30-second windows and each window is assigned a dominant topic/theme.

This produces a topic timeline based only on speech.

The output of this notebook represents the speech-based interpretation of one newscast.

This notebook is expected to be run for the same newscast/date when the user wants to later combine speech and OCR information in notebook 06.

---

# Part 3 — Multimodal and global analysis

The final notebooks combine previous outputs and answer the main research questions of the project.

---

## `06_ocr_speech_fusion.ipynb`

This notebook performs the fusion between the OCR analysis and the speech analysis.

It combines the outputs produced by:

```text
04_single_newscast_ocr_analysis_lowerthird_weighted_consolidated.ipynb
05_single_newscast_speech_30s_topic_analysis.ipynb
```

for the same newscast/date.

The goal is to create a stronger multimodal representation of a newscast by using both:

- visual/OCR information;
- speech/transcription information.

In practice, notebook 06 expects that the OCR output from notebook 04 and the speech output from notebook 05 already exist for the same selected date/newscast.

The workflow is therefore:

```text
04 OCR analysis output
+
05 speech analysis output
        ↓
06 OCR + speech fusion output
```

This notebook is normally run once per date/newscast that we want to include in the later global comparison.

---

## `07_similar_news_across_channels_question8.ipynb`

This notebook answers the research questions related to similar news across RTP and TVI.

It is mainly associated with:

### Question 8

> Is it possible to identify similar news across the two TV channels?

### Question 9

> Do similar news appear in the same time window across the two TV channels?

This notebook uses the fusion outputs produced by notebook 06.

Therefore, to perform a global analysis across several dates, notebook 06 should first be run for all the newscasts/dates that we want to compare.

The dependency is:

```text
06 OCR + speech fusion outputs
        ↓
07 similar news across RTP and TVI
```

Notebook 07 can be used in two ways:

1. **Global comparison**  
   It can compare several processed dates/newscasts, as long as the corresponding fusion outputs from notebook 06 exist.

2. **Case-study comparison**  
   It can compare selected examples manually, for instance one RTP newscast against one TVI newscast, again assuming that both were already processed by notebook 06.

The notebook groups and compares news items across channels using the fused OCR + speech information.

---

## `08_q7_batch_speech_30s_debates_vs_newscasts_v6_same_path_as_05.ipynb`

This notebook answers the research question about the relation between debate topics and newscast topics.

### Question 7

> Can you relate topics discussed in the debates with those identified in the newscasts?

This notebook compares debates and newscasts using a common speech-based methodology.

The main idea is:

- use speech/transcription data for both debates and newscasts;
- split the content into fixed 30-second windows;
- assign a dominant topic/theme to each window;
- aggregate the topic distributions by source type;
- compare debates against newscasts.

This notebook is more global than notebooks 04 and 05.  
It processes multiple speech pickle files and compares the thematic distribution of:

```text
Debates
vs
Newscasts
```

It also allows comparisons such as:

```text
Debates vs RTP newscasts vs TVI newscasts
```

This notebook is mainly speech-based because the debates and the newscasts need to be compared using the same type of information and the same granularity.  
Using speech windows provides a fairer comparison between both types of video.

---

# Summary of notebook roles

| Notebook | Role | Project part |
|---|---|---|
| `01_ocr_structure_quality_FIXED.ipynb` | OCR structure and quality EDA | Part 1 |
| `02_ocr_political_content_cleaned.ipynb` | OCR political content exploration/cleaning | Part 1 |
| `03_ocr_rtp_vs_tvi.ipynb` | Initial RTP vs TVI OCR comparison | Part 1 |
| `04_single_newscast_ocr_analysis_lowerthird_weighted_consolidated.ipynb` | Individual OCR analysis for one newscast | Part 2 |
| `05_single_newscast_speech_30s_topic_analysis.ipynb` | Individual speech/topic analysis for one newscast | Part 2 |
| `06_ocr_speech_fusion.ipynb` | Fusion of OCR and speech outputs for one newscast/date | Part 3 |
| `07_similar_news_across_channels_question8.ipynb` | Similar news across RTP/TVI and time-window comparison | Part 3 |
| `08_q7_batch_speech_30s_debates_vs_newscasts_v6_same_path_as_05.ipynb` | Debate vs newscast topic relation | Part 3 |

---

# Main dependencies

The main dependencies between notebooks are:

```text
04 + 05 → 06 → 07
```

More specifically:

- Notebook 04 produces the OCR analysis output for a selected newscast/date.
- Notebook 05 produces the speech/topic analysis output for a selected newscast/date.
- Notebook 06 fuses the outputs from notebooks 04 and 05 for the same selected newscast/date.
- Notebook 07 uses the fusion outputs from notebook 06 to compare similar news across RTP and TVI.
- Notebook 08 is separate from the OCR/speech fusion pipeline and performs a global speech-based comparison between debates and newscasts.

---

# Recommended execution order

For the complete project workflow, the recommended order is:

```text
01 → 02 → 03
```

for the initial EDA.

Then, for each selected newscast/date:

```text
04 → 05 → 06
```

After notebook 06 has been run for the desired dates/newscasts, notebook 07 can be used to compare similar news across RTP and TVI.

Notebook 08 can be run to answer the debate-vs-newscast topic relation question.

---

# Research questions covered by the final notebooks

The final analysis is organized around three research questions.

## Question 7

> Can you relate topics discussed in the debates with those identified in the newscasts?

Answered by:

```text
08_q7_batch_speech_30s_debates_vs_newscasts_v6_same_path_as_05.ipynb
```

---

## Question 8

> Is it possible to identify similar news across the two TV channels?

Answered by:

```text
07_similar_news_across_channels_question8.ipynb
```

---

## Question 9

> Do similar news appear in the same time window across the two TV channels?

Answered by:

```text
07_similar_news_across_channels_question8.ipynb
```

---

Due to storage limitations, only the most relevant fusion outputs from notebook 06 are included in this submission. These outputs are provided to allow inspection of the multimodal OCR + speech results and to support notebook 07, without requiring all intermediate outputs from notebooks 04 and 05 to be stored.

# Notes

The numbering of Questions 7, 8, and 9 is used here as an internal organization of the final part of the project.

The notebooks 01, 02, and 03 correspond to the initial EDA.  
The notebooks 04 and 05 correspond to individual analysis of one pickle/newscast at a time.  
The notebooks 06, 07, and 08 correspond to the multimodal/global stage of the project.

Notebook 06 is the bridge between the individual OCR/speech analyses and the cross-channel comparison.  
Notebook 07 depends on the outputs produced by notebook 06.  
Notebook 08 is focused on the debate-newscast relation and uses speech windows to make debates and newscasts comparable.
