function data = mem_dump_load(filename)
% Le ficheiro .DTX e devolve os valores das words.
% Esta versao ignora numeros de enderecos e tenta ler o ultimo valor de cada linha.

    if nargin < 1
        error('Indica o nome do ficheiro .DTX');
    end

    fid = fopen(filename, 'r');

    if fid == -1
        error('Nao consegui abrir o ficheiro: %s', filename);
    end

    C = textscan(fid, '%s', 'Delimiter', '\n', 'Whitespace', '');
    fclose(fid);

    lines = C{1};
    data = [];

    for i = 1:length(lines)

        line = strtrim(lines{i});

        if isempty(line)
            continue;
        end

        % Apanha numeros inteiros ou reais
        nums = regexp(line, '[-+]?\d+(\.\d+)?', 'match');

        if isempty(nums)
            continue;
        end

        % Usa apenas o ultimo numero da linha, que normalmente e o valor da word
        value = str2double(nums{end});

        if ~isnan(value)
            data(end+1,1) = round(value); %#ok<AGROW>
        end
    end
end