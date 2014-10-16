vid=videoinput('winvideo',1,'YUY2_320x240');
set(vid,'ReturnedColorSpace','rgb');
triggerconfig(vid, 'manual');
start(vid);