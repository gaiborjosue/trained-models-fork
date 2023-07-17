import os
import subprocess

def get_latest_model_name():
    root_path = os.environ.get('GITHUB_WORKSPACE')

    # Get the commit SHA of the latest push from the GitHub context
    commit_sha = os.environ.get('GITHUB_SHA')

    # Get the list of modified files in the latest push commit
    git_command = ['git', 'diff-tree', '--no-commit-id', '--name-only', '-r', commit_sha]
    modified_files = subprocess.check_output(git_command, cwd=root_path, text=True).splitlines()

    # Find the latest modified model folder
    latest_model_folder = None
    latest_modification_time = 0

    for file in modified_files:
        file_path = os.path.join(root_path, file)
        if os.path.isdir(file_path):
            modification_time = os.path.getmtime(file_path)
            if modification_time > latest_modification_time:
                latest_modification_time = modification_time
                latest_model_folder = file

    if latest_model_folder:
        return os.path.join(root_path, latest_model_folder)
    else:
        return None

if __name__ == "__main__":

    print(get_latest_model_name())