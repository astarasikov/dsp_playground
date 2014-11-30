#!/Applications/Julia.app/Contents/Resources/julia/bin/julia

a = readcsv("mtx_a.csv");
b = readcsv("mtx_b.csv");
c = readcsv("mtx_c.csv");

print(mean(sum(a * b - c)));
