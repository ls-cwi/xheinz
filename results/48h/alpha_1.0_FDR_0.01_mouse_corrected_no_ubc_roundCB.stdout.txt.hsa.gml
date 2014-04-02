graph [
  name "()"
  node [
    id 0
    label "VDR"
    logCPM 6.68777642035
    between_centrality 0.0
    score 8.36514539561
    degree 2
    symbol "VDR"
    ensembl_protein_id "ENSP00000229022"
    ensembl_gene_id "ENSG00000111424"
    conserved 1
    FDR 2.99027383457e-06
    is_relevant 1
    logFC 1.40127189733
  ]
  node [
    id 1
    label "RORC"
    logCPM 6.90081961873
    between_centrality 0.0
    score 14.6521665092
    degree 2
    symbol "RORC"
    ensembl_protein_id "ENSP00000327025"
    ensembl_gene_id "ENSG00000143365"
    conserved 1
    FDR 2.67396012789e-09
    is_relevant 1
    logFC 1.835763905
  ]
  node [
    id 2
    label "RORA"
    logCPM 7.26395221229
    between_centrality 0.0
    score -0.933685436456
    degree 2
    symbol "RORA"
    ensembl_protein_id "ENSP00000261523"
    ensembl_gene_id "ENSG00000069667"
    conserved 1
    FDR 0.0370507685041
    is_relevant 0
    logFC 0.468862769933
  ]
  edge [
    source 0
    target 1
    score 900
  ]
  edge [
    source 0
    target 2
    score 900
  ]
  edge [
    source 1
    target 2
    score 900
  ]
]
