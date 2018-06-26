function [g] = calc_optlrc (n,field,r)

for i=[0:63]
	found=0;
	groups=0;
	base = [];
	val = [];
	bin=de2bi(i);
	bin=fliplr(bin);
	while (length(bin)<= r)
		bin=[0 bin];
    end
	g = [ 1 bin];
	for i=[1:(2^field-1)]
		e= gf (i,field);
		base = [base i];
		val= [val polyval(gf(g,field),e)];
	end
	j=1;
	while(1)
		%count groups of size r+1
		while (sum(val(:) == val(j)) < r+1)
			j=j+1;
			if (j>=length(val))
				break;
			end
		end
		if j<length(val)
			groups = groups+1;
			% remove all found values from array
			val = val(find(val~=val(j)));
		else
			break;
		end
		if (groups == ceil (n/(r+1)))
			return;
		end
	end
end
end