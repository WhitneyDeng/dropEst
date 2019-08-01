Changelog
---------

[0.8.6] - 2019-08-01
~~~~~~~~~~~~~~~~~~~~

Added
^^^^^

- Protocol aliases for Drop-seq, Seq-well and CEL-Seq2

[0.8.5] - 2018-11-14
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Compatibility with preseqR-4.0.0
-  Compatibility with newer boost versions
-  Fixed "merge-umi" cli option in dropest
-  More informative dropEst logging
-  Fixed bug with false warning on "Unexpected chromosome -1"
-  Fixed bug with wrong work of "-F" option with "-u" #### Added
-  dropTag now able to trim and filter gene reads based on quality (see
   ``TagsSearch/Processing`` section of "configs/config\_desc.xml")
-  Pipeline can be installed with ``make install`` now
-  Support for SPLiT-seq protocol (DOI: 10.1126/science.aam8999)

[0.8.3] - 2018-05-17
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Fixed bug with merge failure without barcode file
-  Fixed bug with ``-C`` and ``-u`` options
-  Fixed logs for bam output
-  Fixed output of reads\_per\_umi\_per\_cell
-  Barcodes for 10x chromium v2.0

[0.8.2] - 2018-04-20
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Config field "Estimation/Merge/barcodes\_file" now accepts paths,
   relative to config directory, as well as path with "~/"
-  Fixed bug which perevented passing several files to ``-r`` option of
   dropest (Issue #25)
-  Fixed bug with wrong bam output (``-F`` option)
-  Bam output now saves info about raw barcode sequences, read type and
   barcode quality similarly to 10x CellRanger format
-  Updated dockers for CentOS 6 and Cent OS 7
-  Fixed bug, when UMIs with N nucleotides weren't removed
-  Fixed bug with incorrect processing of read type tag, when it has
   type "A" (Issue #18)
-  Pipeline now uses information from gene bam tag for intergenic reads
   when provided #### Added
-  To improve reproducibility, config file is now copied to the log
   directory
-  'Directional' UMI correction can be applied during dropEst phase with
   "-u" option. In this case, information for more advanced UMI
   correction isn't saved to the output rds.

[0.8.1] - 2018-04-06
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Fixed some bugs in dropestr #### Added
-  Added protocol type 10x (which is alias for indrop3) to dropTag

[0.8.0] - 2018-03-20
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Now, information about reads is kept in separate file instead
   (\*.reads.gz), which should be passed to dropEst
-  New format of *reads\_per\_umi\_per\_cell* in cell.counts.rds

Added
^^^^^

-  UMI correction algorithm now uses UMI quality (only Illumina 1.8 or
   later formats are supported)

[0.7.6] - 2018-02-22
~~~~~~~~~~~~~~~~~~~~

Added
^^^^^

-  Version number output in dropEst #### Changed
-  Fixed bug with report generation
-  Fixed bugs with estimation of low-quality cells

[0.7.5] - 2018-01-28
~~~~~~~~~~~~~~~~~~~~

Added
^^^^^

-  Dockers for Centos6, Centos7 and Debian9
-  iclip protocol support
-  10x barcodes
-  Published code for filtration of multialigned reads from bam files
   with cell mixture. #### Changed
-  Optimized precise merge performance
-  Fixed bug with fastq split during dropTag
-  **New format of barcode files**
-  Optimized memory usage in parsing read params from file
-  Algorithm of filtration of low-quality cells was significantly
   improved

[0.7.1] - 2017-10-18
~~~~~~~~~~~~~~~~~~~~

Added
^^^^^

-  Integration with velocyto

[0.7.0] - 2017-10-17
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Optimized cmake
-  **Secondary alignments are filtered now**

Added
^^^^^

-  Output UMIs with only exonic or only intronic reads

[0.6.8] - 2017-09-22
~~~~~~~~~~~~~~~~~~~~

Added
^^^^^

-  Filtration of reads by barcode quality
   ("*TagsSearch/Processing/min\_barcode\_quality*" and
   "*Estimation/Other/min\_barcode\_quality*" fields in the config)
-  dropEst is now able to parse read type (e.g. exonic/intronic) from
   .bam file (see *config\_desc.xml*)

[0.6.7] - 2017-09-13
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Fixed bug, which led to erroneous parsing of incorrect read (e.g.
   reads without spacer for Indrop V1)

Added
^^^^^

-  Parallelized dropTag ("*-p*" option)

[0.6.5] - 2017-09-07
~~~~~~~~~~~~~~~~~~~~

Changed
^^^^^^^

-  Optimized memory usage and performance of dropEst
-  Sorting for cells selection (by number of genes) is stable now
-  Fixed bug with merge\_targets in low-quality cells estimation
-  Fixed bug with N's in UMIs after the merge

[0.6.1] - 2017-09-05
~~~~~~~~~~~~~~~~~~~~

Added
^^^^^

-  Support for pseudoaligners .bam format (usage of chromosome name as a
   source of gene name)
-  Changelog

Changed
^^^^^^^

-  Check R libraries immediately after dropEst start

[0.6.0] - 2017-09-04
~~~~~~~~~~~~~~~~~~~~

Added
^^^^^

-  Versioning
