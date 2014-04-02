graph [
  name "()"
  node [
    id 0
    label "STAT3"
    logCPM 9.45816949325
    between_centrality 0.0
    score 5.36941125611
    degree 1
    symbol "STAT3"
    ensembl_protein_id "ENSP00000264657"
    ensembl_gene_id "ENSG00000168610"
    conserved 0
    FDR 1.22555796613e-07
    is_relevant 1
    logFC 1.38440907811
  ]
  node [
    id 1
    label "ELAVL1"
    logCPM 7.02344862269
    between_centrality 1.0
    score -3.82866730835
    degree 3
    symbol "ELAVL1"
    ensembl_protein_id "ENSP00000385269"
    ensembl_gene_id "ENSG00000066044"
    conserved 0
    FDR 0.65611918271
    is_relevant 0
    logFC -0.190697580333
  ]
  node [
    id 2
    label "PFKFB3"
    logCPM 9.70549378695
    between_centrality 0.0
    score 3.02026800989
    degree 1
    symbol "PFKFB3"
    ensembl_protein_id "ENSP00000369100"
    ensembl_gene_id "ENSG00000170525"
    conserved 0
    FDR 1.24350057089e-05
    is_relevant 1
    logFC 1.15222668167
  ]
  node [
    id 3
    label "FVT1"
    logCPM 8.98798310684
    between_centrality 0.0
    score 2.06674210584
    degree 1
    symbol "FVT1"
    ensembl_protein_id "ENSP00000385083"
    ensembl_gene_id "ENSG00000119537"
    conserved 0
    FDR 9.49079495615e-05
    is_relevant 0
    logFC 0.862980585167
  ]
  edge [
    source 2
    target 1
    score 214
  ]
  edge [
    source 0
    target 1
    score 214
  ]
  edge [
    source 1
    target 3
    score 214
  ]
]
