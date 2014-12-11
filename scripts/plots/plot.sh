set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5 
set title "Performance with load balancing enabled"
set ylabel "Number of queries"
set xlabel "Time in sec"
set xrange [0:85]
set yrange [0:150]
set term png
set output 'timeVsLoad.png'
plot 'timeVsLoad.dat' with linespoint ls 1

