
# No debe existir new_dir

echo -n "04_empty_dir_reading_with_ls - "


mkdir new_dir

num=$(ls -1 -a new_dir | wc -l)

if [[ "$num" == "2" ]]
then
    echo -e "\033[1;32mOK\033[0m"
else
    echo -e "\033[1;31mFAIL\033[0m"
fi

rmdir new_dir
