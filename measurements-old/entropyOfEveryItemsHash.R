# Zuweisung von Hashfunktion zu jedem Element, Entropie davon, summe.
# Mit perfekter Codierung könnte man somit 2.3 bits pro Element speichern
# Wenn man pro Element die erste Hashfunktion nimmt, die geht.

#0.5+a+b=1
#0.5*2+a*4+b*8=4
#https://ieeexplore.ieee.org/abstract/document/4690964
#parameter für das i-te Element:
#  p ist Erfolgswahrscheinlichkeit
#(m-i+-1)/m
#p=(m-i+1)/m


m <- 100000
p <- function(i) { (m-i+1)/m }
i <- seq(from = 1, to = m, by = 1)
b <- (-(1-p(i))*log2(1-p(i)) - p(i)*log2(p(i))) / p(i)
b[1] <- 0
s <- sum(b)
print(s/m)
