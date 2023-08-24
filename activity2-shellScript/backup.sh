# This code is to create a directory backup. 
# It should create a directory called backup and copy all the files from the current directory into the backup directory.
# It should also create a log file called backup.log and write when the date of the backup, the time it started, 
# a list of the files that were copied, and the time it finished.

#! /bin/bash
backupDir=/home/ec2-user/backups
fileDirToCopy=/home

# Creating backup directory if it doesn't exist
if [ ! -d "$backupDir" ]; then
    mkdir $backupDir
fi

# Initial Log information
date=$(date)
dateInUnderscoreFormat=$(date +%d_%m_%Y)
dayMonthYearDate=$(date +%d/%m/%Y)
backupLog=$backupDir/backup_$dateInUnderscoreFormat.log
touch $backupLog

echo -e "Execução do backup: $dayMonthYearDate" >> $backupLog
timeOfStart=$(date +%H:%M:%S)
echo -e "Horário de início - $timeOfStart\n" >> $backupLog

# Archive file name
archiveFileName="backup_$dateInUnderscoreFormat.tgz"

# Using tar to create a compressed file
tar czf $backupDir/$archiveFileName $fileDirToCopy

# Getting the compressed files
echo -e "Arquivos compactados:" >> $backupLog
compactedFile=$(tar -tf $backupDir/$archiveFileName)
echo -e "$compactedFile\n" >> $backupLog

# Final Log information
echo -e "Diretório de backup: $backupDir" >> $backupLog
timeOfEnd=$(date +%H:%M:%S)
echo -e "Horário da Finalização - $timeOfEnd\n" >> $backupLog