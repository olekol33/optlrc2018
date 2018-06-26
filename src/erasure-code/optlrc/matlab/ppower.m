function [new_pol] = ppower (pol, p)
	i=0;
	new_pol = [1];
	while i<p
		new_pol = conv(new_pol,pol);
		i=i+1;
	end
	if p==0
		new_pol=[1];
	end
end