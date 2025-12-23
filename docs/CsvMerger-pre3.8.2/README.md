CsvMerger notes, May 7, 2025 - pre 3.8.2, M. Picciau

This component performed post-processing of the csv (merging the thread-specific CSVs)
in all versions up to 3.8.1. It has been removed in 3.8.2, as it does not perform well
with high volumes, and also architecturally I ended up preferring giving up the line
of in-line postprocessing. No further attempts will be made in this area, and postprocessing tools
will be provided instead.

