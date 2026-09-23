
### Multimodal OCR + Speech Fusion Summary

The final newscast segmentation uses OCR as the temporal backbone because OCR captures visual lower-thirds, titles and anchor-to-piece transition candidates.
Speech is aligned with these OCR blocks using temporal overlap and is used to validate or enrich the topic assigned to each block.

- OCR blocks analysed: **107**
- Blocks with high OCR/speech agreement: **12**
- Blocks with related/medium agreement: **1**
- Blocks with low conflict: **13**
- Blocks marked for manual review: **90**
- Main final themes: **Other/Unknown, Justiça/Segurança, Economia/Proteção Social, Saúde, Internacional**

Methodological interpretation:

- OCR defines the estimated news boundaries and duration.
- Speech validates whether the spoken content matches the OCR lower-third topic.
- When OCR and speech agree, confidence in the block topic increases.
- When they disagree, the block is flagged for manual review rather than automatically changing the OCR boundary.
- Anchor-to-piece transitions remain OCR-based evidence, because the speech pipeline does not directly detect visual anchor-to-piece scene changes.
