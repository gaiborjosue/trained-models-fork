import os

def get_latest_model_name():
# Get the root of the repository using the GITHUB_WORKSPACE environment variable
    root_path = os.environ.get('GITHUB_WORKSPACE')

    os.chdir(root_path)

    org_folder = max([folder for folder in os.listdir('.') if os.path.isdir(folder)], key=os.path.getctime)
    
    model_path = os.path.join(root_path, org_folder)

    model_folder = max([folder for folder in os.listdir(model_path) if os.path.isdir(os.path.join(model_path, folder))], key=os.path.getctime)

    return os.path.join(model_path, model_folder)

if __name__ == "__main__":

    print(get_latest_model_name())