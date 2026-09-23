function unity_string_make( str, addr, options )

% Write a string as UnityPro Structure Text (ST) code.
% The code overwrites an array of 16bit words.
% Note: If not an even number of characters then add a space at the end.

% Example of a simple string:
% unity_string_make( '123abc' );
% Example of an extra long string:
% unity_string_make( char( rem(0:79,10)+'0' ), [], struct('printMode',2) )

% March2020, JG

if nargin<1
    str= 'Hello world!';
end
if nargin<2 || isempty(addr)
    addr= 190;
end
if nargin<3
    options= [];
end
printMode= 1;
if isfield(options, 'printMode'), printMode= options.printMode; end

% enforce even number of characters
if rem(length(str),2)~=0
    str= [str ' '];
end

fid= 1;
if isfield(options, 'ofname')
    fid= fopen( options.ofname, 'wt' );
end

% write to the screen the ST code
fprintf(fid, '(* string "%s" in ST *)\n', str);
x0= [];
for i=1:2:length(str)
    if printMode==0
        % show 16bits as one single decimal number
        x= str(i)+256*str(i+1);
        fprintf(fid, '%%MW%d:=%d;\n', addr, x);
    elseif printMode==1
        % show separated ASCII codes (2 in a 16bits word)
        % instead of "256*(%03d)" one can use "shl(%d,8)"
        fprintf(fid, '%%MW%d:= (%03d) +256*(%03d);\n', addr, str(i), str(i+1));
    elseif printMode==2
        % show as 32 values (use double words)
        x= str(i)+256*str(i+1);
        if isempty(x0)
            x0= x;
        else
            x= x0 +256*256*x;
            fprintf(fid, '%%MD%d:= 16#%s;\n', addr-1, dec2hex(x,8));
            x0= [];
        end
    else
        error('inv printMode');
    end
    addr= addr+1;
end

if ~isempty(x0)
    % str length not a multiple of 4 bytes
    %fprintf(fid, '%%MW%d:=%d;\n', addr, x0);
    %fprintf(fid, '%%MW%d:= 16#%s;\n', addr, dec2hex(x0,4));
    %addr= addr+1;
    fprintf(fid, '%%MD%d:= 16#%s; (* is null terminated *)\n', addr-1, dec2hex(x0,8));
else
    % null terminating string
    fprintf(fid, '%%MW%d:= 0;\n', addr);
end

if fid>1
    fclose(fid);
end
