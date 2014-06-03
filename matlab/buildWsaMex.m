function buildWsaMex()

    currentDirectory = pwd;
    [mFilePath, ~, ~] = fileparts(mfilename('fullpath'));
    cd(mFilePath);

    try
        if exist('buildVersion.mat', 'file') == 2
            load('buildVersion.mat');
            if ~strcmp(mexBuildVersion, version)
                delete(strcat('*.', mexext));
            end
        end

        if isempty(strfind(mexext, '64'))
            wsaLibName = 'libwsa32.a';
        else
            wsaLibName = 'libwsa64.a';
        end

        cppFiles = ls('*.cpp');
        [numberOfFiles ~] = size(cppFiles);
        for i = 1 : numberOfFiles
            buildWsaMexFile(strtrim(cppFiles(i, :)), wsaLibName);
        end

        mexBuildVersion = version;
        save('buildVersion.mat', 'mexBuildVersion');
    catch err
        cd(currentDirectory);
        rethrow(err);
    end

    cd(currentDirectory);

end


function buildWsaMexFile(filename, wsaLibName)

    % Strip off the extension (.c or .cpp)
    [~, bareFilename, ~] = fileparts(filename);

    % This is what the mex file will be called:
    mexFilename = strcat(bareFilename, '.', mexext);

    % Build the mex file if it does not exist already
    if exist(mexFilename, 'file') ~= 3
        disp(['Building "' mexFilename '". This should take less than a minute.']);
        mex(filename, wsaLibName, 'ws2_32.lib', '-I../api/include');
    end

end
