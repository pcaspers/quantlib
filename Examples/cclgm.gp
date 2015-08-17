unset key
plot for [n=0:99] 'mcpaths.dat' u 1:(column(2+3*n)) w l
plot for [n=0:99] 'mcpaths.dat' u 1:(column(3+3*n)) w l
plot for [n=0:99] 'mcpaths.dat' u 1:(column(4+3*n)) w l

# fx process
plot for [n=0:0] 'mcpaths.dat' u 1:(column(2+3*n)) w l

# eur and usd lgm process
plot for [n=0:0] 'mcpaths.dat' u 1:(column(3+3*n)) w l lt 1, for [n=0:1] '' u 1:(column(4+3*n)) w l lt 2