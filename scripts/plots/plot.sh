set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5 
set title "Performance with load balancing"
set ylabel "Number of queries"
set xlabel "Time in sec"
set xrange [0:80]
set yrange [0:150]
set term png
set output 'loadBalance.png'
plot 'loadBalance.dat' with linespoint

