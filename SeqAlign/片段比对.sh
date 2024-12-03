# 将查询序列按指定大小分割，例如每个片段长度为100bp
bedtools split -i query.txt -n 100 -p query_fragments

# 按滑动窗口分割序列，窗口大小100bp，步长50bp
seqkit sliding -s 50 -W 100 query.txt -o query_fragments.fasta

bwa index chr6 -p chr6

bwa mem -t 8 chr6 query_fragments.fasta > align_fragments.sam

samtools view -bS align_fragments.sam > align_fragments.bam

samtools sort align_fragments.bam -o align_fragments.sorted.bam

# 过滤 MAPQ 值低于 30 的比对
samtools view -h -q 30 align_fragments.sorted.bam > align_fragments.filtered.sam

bedtools bamtobed -i align_fragments.filtered.sam > align_fragments.bed

# 提取比对信息
awk '{if($0 !~ /^@/){print $1, $4, $6}}' align_fragments.filtered.sam > cigar_info.txt