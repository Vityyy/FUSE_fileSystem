
# Debe existir un archivo llamado 'example_file' con el contenido 'hola fisopfs!'

echo -n "09_file_deletion_with_rm - "


cp example_file tmp
rm example_file

ls -1 | grep -q example_file

if [ $? -ne 0 ]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

cp tmp example_file
rm tmp
