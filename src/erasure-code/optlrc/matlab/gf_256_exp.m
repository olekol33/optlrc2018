%This is the code:
f1 = gf([0 0 0 0 1],8)
f2 = gf([0 0 0 1 0],8)
f3 = gf([0 1 0 0 0],8)
f4 = gf([1 0 0 0 0],8)
g = [;]
primel = gf(2,4)
%set of local groups
for i = [214   215     1     2   177   179     4   127   123]
k= gf (i,8)
%logarithmic to binary value (used for calculations)
%j = primel.^k
j= k
%evaluate in poly
b = [polyval(f1,j) polyval(f2,j) polyval(f3,j) polyval(f4,j)]
%convert back to log
g= [g;b]
end
g=g'

%logarithmic to binary value (used for calculations)
%gint = primel.^g
%take k independent columns
ginti = g(:,[1 2 4 5])
%invert matrix
gintiinv = inv(ginti)
%multiply to generate systematic matrix and convert to log type
glob=gintiinv*g
for i=1:254
primeli=gf(i,8)
for j=1:254
primelj=gf(j,8)
for k=1:254
primelk=gf(k,8)
a = primeli*glob(:,[1])+primelj*glob(:,[2])+primelk*glob(:,[3])
if (a==0)
index = [i j k]
break
end
end
if (a==0)
    break
end
end
if (a==0)
    break
end
end

