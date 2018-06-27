function [encode,perm,coef] = calc_optlrc (n,k,r)
field=8;
%can't have local group of size 1
if (mod(n,r+1) == 1)
	error('s=1 illegal\n');
end

i=1;
found=0;
%determine whether the subgroup is multiplicative or additive and construct g(x)
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
%for r=5
if (r==5)
    g_old=g_for_r_5(n,field,r);
    found=1;
    fprintf('r=5 subgroup\n');
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
flaga=0;
grAmt = [];
grAt = [];
s = mod(n,r+1);
t=r+1-s;
tempt=t;
while count>0

	%when r+1 no dividing n
	if count == mod(n, r+1)
		count_loc= mod(n, r+1)-1;
		init_group = mod(n, r+1);
		flaga=1;
	else
		count_loc=r;
		init_group = r+1;
	end
	
	%take groups of required minimal size
	while (sum(val(:) == val(i)) < r+1)
		i=i+1;
	end

	%already in
	while sum(locations(:) == base(i)) > 0
		i=i+1;
	end
	same = val(i);
	locations = [locations base(i)];
    %take  r+1-t = s values into Amt
	if (flaga==1)
		grAmt= [ grAmt base(i) ];
	end
	for j = [i+1: 2^field-1]
		if val(j) == same & count_loc>0
			locations = [locations base(j)];
			count_loc = count_loc-1;
			if (flaga==1)
				grAmt= [ grAmt base(j) ];
			end
		end
		if count_loc == 0
			count = count - init_group;
			j = j+1;
			if (flaga == 1)
				if val(j) == same
                    % take t values into At (but not into locations)
					grAt = [ grAt base(j) ];
					tempt = tempt-1;
					if (tempt==0)
						flaga=0;
					end
				end
			else
				break;
			end
		end
	end
	i=i+1;
end


%build polynomial
locations = gf(locations,field);
grAt = gf(grAt,field);
grAmt = gf(grAmt,field);
if mod(n,r+1) > 0
	kst=k+t;
	%h is null on element of At
	h = [1];
	for i=[1:t]
		p = [1 0] + [0 grAt(i)]
		h = conv (h,p);
	end
	
	%new g is shifted to zero by element of A
	g_el = zeros(1,r+1);
	g_el = [g_el polyval(gf(g_old,field),grAmt(1))];
	g = g_old + g_el;
	
	%max degree of fa
	%max_len=k-1+(k+1)/r;
	max_len=floor(kst/r)*(r+1)+(mod(kst,r)-1);
	
	%calculate part 1/2
	mulg = [1];
	mulx = [1];
	
	M=[;];
	for i=[0:r-1]
		if i < mod(kst,r)
			for j=[1:floor((kst)/r)]
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
			for j=[1:(floor((kst)/r)-1)]
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
	
	
	%calculate part 2/2	
	mulg = [1];
	mulx = [1];
	for i=[0:r-1-t]
		mulx=ppower([1 0],i);
		res=conv(h,mulx);
		%pad to same length
		while (length(res)<= max_len)
			res=[0 res];
		end
		M=[M ; res];
	end
	

	
else

	max_len=k+ceil(k/r)-2;
	M=[;];
	mulg = [1];
	mulx = [1];
	for i=[0:r-1]
        if i < mod(k,r)
            for j=[0:floor(k/r)]
                mulg=ppower(g_old,j);
                mulx=ppower([1 0],i);
                res=conv(mulg,mulx);
                while (length(res)<= max_len)
                    res=[0 res];
                end
                res
                M=[M ; res]
                %fa=(gsym^j)*(x^i)+fa
            end
        else
            for j=[0:floor(k/r)-1]
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
        %redun_coef = calc_optlrc_coef(A,r,field,n,locations);
        redun_coef=[ 214   109    43    70     1];
		break;
	else
		count_loc=r+1;
		for i=[1:count_loc]
			A=[A mat(:,group*(r+1)+i)];
        end
        if (group==0)
            coef = [coef;  215,    49,   232,   217 ,  214 ,    1];
         elseif (group==1)
             coef=[coef; 214  , 102   ,212 ,  178 ,  215  ,   1];
        else 
        coef=[coef; calc_optlrc_coef(A,r,field,n,locations)];
            end
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

% index = [];
% while 1
% 	%randomly select k vector until matrix is invertible
% 	index = [];
% 	b=[;];
% 	for i=1:k
% 		ranval = floor( mod(1000*rand,n))+1;
% 		while ( ismember (ranval,index) ~= 0)
% 			ranval = floor( mod(1000*rand,n))+1;
% 		end
% 		index= [ index  ranval];
% 		b = [b, mat(:, ranval) ];
% 	end
% 	if size(b,2) == k
% 		if (det(b) ~= 0 )
% 			break
% 		end
% 	end
% end
b= [;];
i=1;
ind=1;
while i<=k
	if mod(ind,r+1) == 0
		ind=ind+1;
	end
	b = [b, mat(:, ind) ];
	ind=ind+1;
	i=i+1;
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
%encode
%encode=encode';
encode=glob';

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
fprintf(fileID,"}\n");
% fprintf(fileID,"optlrc_perm : {");
% for i=[1:n-1]
%      fprintf(fileID,"%d,",perm(i)-1);
% end
% fprintf(fileID,"%d}\n",perm(n)-1);
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


