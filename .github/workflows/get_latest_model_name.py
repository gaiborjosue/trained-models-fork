import os

def get_latest_model_name():
    # Get the root of the repository using the GITHUB_WORKSPACE environment variable
    root_path = os.environ.get('GITHUB_WORKSPACE')

    with os.scandir(root_path) as entries:
        org_folder = max((entry.name for entry in entries if entry.is_dir()), key=os.path.getctime)

    model_path = os.path.join(root_path, org_folder)

    with os.scandir(model_path) as entries:
        model_folder = max((entry.name for entry in entries if entry.is_dir()), key=os.path.getctime)

    return os.path.join(model_path, model_folder)

if __name__ == "__main__":

    print(get_latest_model_name())