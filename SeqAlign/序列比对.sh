bwa index -p chr6 chr6_fragment

# -t 8: number of threads
bwa mem -t 8 chr6 ../query.txt > align.sam

samtools view -bS align.sam > align.bam

samtools sort align.bam -o align.sorted.bam

bedtools bamtobed -i align.sorted.bam > align.bed

