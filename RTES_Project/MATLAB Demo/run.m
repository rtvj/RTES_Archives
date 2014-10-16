clearvars -except vid
cc=input('Enter the color\n1.RED\n2.GREEN\n3.BLUE\n');
th=input('Enter the Threshold\n');
b1(240,320)=uint8(0);
b2=b1;
H = fspecial('average', [3 3]);
figure('units','normalized','outerposition',[0 0 1 1])
b3(240,320,3)=uint8(0);
 while (1)
    
   data=getsnapshot(vid);
   data=double(data);
   data(:,:,1)=fliplr(data(:,:,1));
   data(:,:,2)=fliplr(data(:,:,2));
   data(:,:,3)=fliplr(data(:,:,3));
   b3(:,:,cc)=uint8(data(:,:,cc));
   bx=(0.2989*data(:,:,1)+0.5870*data(:,:,2)+0.1140*data(:,:,3));
   b1 = (data(:,:,cc)- bx);
   b1=uint8(b1);
   bx=uint8(bx);
   b1 = imfilter(b1, H);
   
   x=th+sum(sum(b1))/numel(b1);
  
   inTarget=0;
   xcnt=0;
   ycnt=0;
   xcntprev=0;
   ycntprev=0;
   xcent=160;
   ycent=120;
 
   xl=0;
   xr=0;
   yl=0;
   yr=0;
   
   for i=1:240
       for j=1:320
           
           if not(inTarget) && b1(i,j)>x
              inTarget=1;
              xcnt=0;
              xl=j;   
           elseif inTarget && b1(i,j)<x
               inTarget=0;
               xr=j;
               if xcnt>xcntprev
               xcent=round(xl+((xr-xl)/2));
               end
               xcntprev=xcnt;
           elseif inTarget==1
               xcnt=xcnt+1;
           end
         
           if b1(i,j)>x
               b2(i,j)=255;
           else
               b2(i,j)=0;
           end 
       end
   end
   
   
   
   
   for j=1:320
       for i=1:240
      
           if not(inTarget) && b1(i,j)>x
              inTarget=1;
              ycnt=0;
              yl=i;   
           elseif inTarget && b1(i,j)<x
               inTarget=0;
               yr=i;
               if ycnt>ycntprev
               ycent=round(yl+((yr-yl)/2));
               ycntprev=ycnt;
               end
           elseif inTarget
               ycnt=ycnt+1;
           end
         
       end
   end
   
   
   data=uint8(data);
   b1=uint8(b1);
   b1(:,xcent:xcent+1,:)=255;
   b1(ycent:ycent+1,:,:)=255;
   b2(:,xcent:xcent+1,1)=255;
   b2(ycent:ycent+1,:,1)=255;
   subplot(2,2,1)
   imshow(data)
   subplot(2,2,2)
   imshow(b3)
   subplot(2,3,4)
   imshow(bx)
   subplot(2,3,5)
   imshow(b1)
   subplot(2,3,6)
   imshow(b2)
   drawnow;
       
end