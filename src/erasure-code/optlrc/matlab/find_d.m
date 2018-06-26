function [d] = find_d (encode, k,n)
% failed=1;
% while 1
%     failed=failed+1;
%     erased=1:failed;
%     mat=encode(:,k+1:n);
%     for i=[1:failed]
%         for j=[1:k]
%             loc=[;];
%             for p=[1:k]
%                 if (ismember(p,erased))
%                     loc = [loc ; mat(p,:)];
%                 end
%             end
%             if rank(loc)<failed
%                 erased
%                 fprintf('d is %d\n',failed);
%                 return;
%             elseif rank(loc) == n-k
%                 fprintf('d = %d  -  MDS!!!\n',n-k);
%                 return;
%             end
%             if ismember(j,erased)
%                 continue;
%             end
%             erased(i)=j;
%         end
%     end
% end


failed=1;
iterations=0;
erased=[];
mat=encode(:,k+1:n);
while 1

        failed=failed+1;
        erased=1:failed;
        %for j=[1:k]
        while 1
            erased
            if (length(unique(erased)))==length(erased)
                loc=[;];
                for p=[1:k]
                    if (ismember(p,erased))
                        loc = [loc ; mat(p,:)];
                    end
                end
                if rank(loc)<failed
                    fprintf('d is %d\n',failed);
                    return;
                elseif rank(loc) == n-k && length(unique(erased))==1 && erased(1)==k
                    fprintf('d = %d  -  MDS!!!\n',n-k+1);
                    return;
                end    
            end

         if erased(failed) == k
            if (length(unique(erased))==1) && (erased(1)==k)
                break;
            end
                erased(failed)=1;
                %for i=[1:(failed-1)]
                i=1;
                while 1
                    while (erased(failed-i) == k)
                        i=i+1;
                        if (failed-i) == 0
                            break;
                        end
                    end
                    if (failed-i) == 0
                    	break;
                    end
                    erased(failed-i)=erased(failed-i)+1;
                    i=i-1;
                    while i>=0
                        erased(failed-i)=1;
                        i=i-1;
                    end
                    break;
                end
         end
         if erased(failed)<k
            erased(failed)=erased(failed)+1;
         end
        end
end
end

     
     

     
     
