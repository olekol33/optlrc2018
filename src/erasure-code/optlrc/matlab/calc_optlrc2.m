function [encode,perm,coef] = calc_optlrc2 (n,k,r)
field=8;

if (mod(n,r+1) == 1)
	error('s=1 illegal\n');
end

i=1;
found=0;
while (i<=r+1)
	if (r+1 == i)
		found=1;
		fprintf('Multiplicative subgroup\n');
		for i=[0:r-1]
			g_old = ppower([1 0], r+1);
		end
		break;
	end
	i=i+2;
end
if found == 0
	i=1;
	while ((2^i) <= r+1)
		if (r+1 == (2^i))
			found=1;
			fprintf('Additive subgroup\n');
			g_old=[1];
			for j=[0:r]
				max_len=3;
				coef=de2bi(j);
				coef=flip(coef);
				while (length(coef)< max_len)
					coef= [0 coef];
				end
				subel = coef(3)*(gf(1,field))+coef(2)*(gf(2,field))+coef(1)*(gf(4,field));
				g_old=conv(g_old,[1 subel]);
			end
			break;
		end
		i=i+1;
	end
end
if (found ==0)
	error('No subgroup\n');
end


%Calculate locations
val = [];
base = [];
for i=[1:(2^field-1)]
	e= gf (i,field);
	base = [base i];
	%val= [val,e^(r+1)];
	val= [val polyval(gf(g_old,field),e)];
end
count=n;
i=1;
locations = [];

while count>0

	%when r+1 no dividing n
	if count == mod(n, r+1)
		count_loc= mod(n, r+1)-1;
		init_group = mod(n, r+1);
	else
		count_loc=r;
		init_group = r+1;
	end
	
	%take groups of required minimal size
	while (sum(val(:) == val(i)) < init_group)
		i=i+1;
	end

	%already in
	while sum(locations(:) == base(i)) > 0
		i=i+1;
	end
	same = val(i);
	locations = [locations base(i)];

	for j = [i+1: 2^field-1]
		if val(j) == same
			locations = [locations base(j)];
			count_loc = count_loc-1;
		end
		if count_loc == 0
			count = count - init_group;
			break;
		end
	end
	i=i+1;
end


%build polynomial
locations = gf(locations,field);
if mod(n,r+1) > 0
	s = mod(n,r+1);
	
	%h is null on element of Am
	h = [1];
	for i=[0:s-1]
		p = [1 0] + [0 locations(n-i)];
		h = conv (h,p);
	end
	
	%new g is shifted to zero by element of Am
	g_el = zeros(1,r+1);
	g_el = [g_el polyval(gf(g_old,field),locations(n))];
	g = g_old + g_el;
	
	
	%max degree of fa
	%max_len=k-1+(k+1)/r;
	max_len=k-1+floor((k+1)/r)+1;
	
	%calculate part 1/3
	mulg = [1];
	mulx = [1];
	
	M=[;];
	for i=[0:s-2]
		if i < mod(k+1,r)
			for j=[0:floor((k+1)/r)]
				mulg=ppower(g,j);
				mulx=ppower([1 0],i);
				res=conv(mulg,mulx);
				%pad to same length
				while (length(res)<= max_len)
					res=[0 res];
                end
				M=[M ; res];
			end
		else
			for j=[0:(floor((k+1)/r)-1)]
				mulg=ppower(g,j);
				mulx=ppower([1 0],i);
				res=conv(mulg,mulx);
				%pad to same length
				while (length(res)<= max_len)
					res=[0 res];
				end
				M=[M ; res];
			end
		end
	end
	
	
	%calculate part 2/3	
	mulg = [1];
	mulx = [1];
	if s-1 < mod(k+1,r)
		for j=[1:floor((k+1)/r)]
			mulg=ppower(g,j);
			mulx=ppower([1 0],s-1);
			res=conv(mulg,mulx);
			%pad to same length
			while (length(res)<= max_len)
				res=[0 res];
			end
			M=[M ; res];
		end
	else
		for j=[1:(floor((k+1)/r)-1)]
			mulg=ppower(g,j);
			mulx=ppower([1 0],s-1);
			res=conv(mulg,mulx);
			%pad to same length
			while (length(res)<= max_len)
				res=[0 res];
			end
			M=[M ; res];
		end
	end
	
	
	%calculate part 3/3
	mulg = [1];
	mulx = [1];
	if (r-1 >= s)
		for i=[s:r-1]
			if i < mod(k+1,r)
				for j=[0:floor((k+1)/r)]
					mulg=ppower(g,j);
					mulx=ppower([1 0],i-s);
					res=conv(mulg,mulx);
					res2=conv(res,h);
					%pad to same length
					while (length(res2)<= max_len)
						res2=[0 res2];
					end
					M=[M ; res2];
				end
			else
				for j=[0:(floor((k+1)/r)-1)]
					mulg=ppower(g,j);
					mulx=ppower([1 0],i-s);
					res=conv(mulg,mulx);
					res2=conv(res,h);
					%pad to same length
					while (length(res2)<= max_len)
						res2=[0 res2];
					end
					M=[M ; res2];
				end
			end
		end
	end

else

	max_len=k+(k/r)-2;
	M=[;];
	mulg = [1];
	mulx = [1];
	for i=[0:r-1]
		for j=[0:(k/r)-1]
			mulg=ppower(g_old,j);
			mulx=ppower([1 0],i);
			res=conv(mulg,mulx);
			while (length(res)<= max_len)
				res=[0 res];
			end
			M=[M ; res];
			%fa=(gsym^j)*(x^i)+fa
		end
	end
end
%log(locations)
mat = [;];
%set of local groups
for i = locations
	b= [;];
	%evaluate in poly
	for m=[1:k]
		b = [b;polyval(gf([M(m,:)],field),i)];
	end
	mat= [mat,b]
end
%bring generator matrix to coefficients 1
count =n;
coef=[;];
group=0;
while (count > 0)
	A=[;];
	%when r+1 no dividing n
	if count == mod(n,r+1)
		count_loc= mod(n,r+1);
		for i=[1:count_loc]
			A=[A mat(:,group*(r+1)+i)];
        end
        redun_coef = calc_optlrc_coef(A,r,field,n,locations);
		break;
	else
		count_loc=r+1;
		for i=[1:count_loc]
			A=[A mat(:,group*(r+1)+i)];
        end
		coef=[coef; calc_optlrc_coef(A,r,field,n,locations)];
		count=count-count_loc;
		group=group+1;
	end
end	
mat2=mat;
for i=1:(n-mod(n,r+1))
	mat(:,i)=mat(:,i)*(coef(ceil(i/(r+1)),1+mod(i-1,r+1)));
end
if mod(n,r+1) > 0
	for i=1:(mod(n,r+1))
		mat(:,i+n-mod(n,r+1))=mat(:,i+n-mod(n,r+1))*(redun_coef(i));
	end
end
%collect k independent columns - at least one 
% vector from each group
b= [;];
index = [];
while 1
	%randomly select k vector until matrix is invertible
	index = [];
	b=[;];
	for i=1:k
		ranval = floor( mod(1000*rand,n))+1;
		while ( ismember (ranval,index) ~= 0)
			ranval = floor( mod(1000*rand,n))+1;
		end
		index= [ index  ranval];
		b = [b, mat(:, ranval) ];
	end
	if size(b,2) == k
		if (det(b) ~= 0 )
			break
		end
	end
end
%invert matrix
gintiinv = inv(b);
%multiply to generate systematic matrix
glob=gintiinv*mat

%calculate coefficients
% count =n;
% coef=[;];
% group=0;
% while (count > 0)
% 	A=[;];
% 	%when r+1 no dividing n
% 	if count == mod(n,r+1)
% 		count_loc= mod(n,r+1);
% 		for i=[1:count_loc]
% 			A=[A glob(:,group*(r+1)+i)];
%         end
%         redun_coef = calc_optlrc_coef(A,mod(n,r+1)-1,field);
% 		break;
% 	else
% 		count_loc=r+1;
% 		for i=[1:count_loc]
% 			A=[A glob(:,group*(r+1)+i)];
% 		end
% 		coef=[coef; calc_optlrc_coef(A,r,field)];
% 		count=count-count_loc;
% 		group=group+1;
% 	end
% end	

%calculate permutations
perm=[];
for i=[1:n]
	perm=[ perm i];
end
encode=glob;
for i=[1:k]
	vec=zeros(1,k);
	vec(i)=1;
	vec=gf(vec',field);
	for j=[1:n]
		if (vec == encode(:,j))
			encode(:,[i,j]) = encode(:,[j,i]);
			perm([i j]) = perm([j i]);
			break;
		end
	end
end

%find_d(encode,k,n)
encode
encode=encode';

%print matrices
fileID = fopen('matrices.txt','w');
fprintf(fileID,"struct OptLRC optlrc_%d_%d_%d = {\n",n,k,r);
fprintf(fileID,"optlrc_encode : {\n");
for i=[1:n-1]
    temp=encode(i,1);
	fprintf(fileID,"{%d",temp.x);
	for j=[2:k];
        temp=encode(i,j);
		fprintf(fileID,", %d",temp.x);
	end
	fprintf(fileID,"},\n");
end
temp=encode(n,1);
fprintf(fileID,"{%d",temp.x);
for j=[2:k]
    temp=encode(n,j);
	fprintf(fileID,", %d",temp.x);
end
fprintf(fileID,"}\n");
fprintf(fileID,"},\n");
fprintf(fileID,"optlrc_perm : {");
for i=[1:n-1]
     fprintf(fileID,"%d,",perm(i)-1);
end
fprintf(fileID,"%d}\n",perm(n)-1);
% fprintf(fileID,"optlrc_coef : {\n");
% for i=1:(floor(n/(r+1))-1)
%     len=r+1;
%     fprintf(fileID,"{");
%     for j=1:len-1
%         %temp=coef(i,j);
%         fprintf(fileID,"%d,",coef(i,j));
%     end
%     fprintf(fileID,"%d},\n",coef(i,r+1));
% end
%     len=r+1;
%     fprintf(fileID,"{");
%     for j=1:len-1
%         temp=coef(i,j);
%         fprintf(fileID,"%d,",coef(i,j));
%     end
%     if (mod(n,r+1) == 0)
%         fprintf(fileID,"%d}\n}",coef(i,floor(n/(r+1))));
%     else
%         fprintf(fileID,"%d},\n}",coef(i,floor(n/(r+1))));
%         len=length(redun_coef);
%         fprintf(fileID,"{");
%         for j=1:len-1
%             %temp=coef(i,j);
%             fprintf(fileID,"%d,",coef(i,j));
%         end
%         fprintf(fileID,"%d}\n",coef(i,r+1));
%     end
   fprintf(fileID,"};");
fclose(fileID);

fprintf("done\n");
end


