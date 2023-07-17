import os

def get_latest_model_name():
    org_folder = max([folder for folder in os.listdir('.') if os.path.isdir(folder)], key=os.path.getctime)
    model_folder = max([folder for folder in os.listdir(org_folder) if os.path.isdir(os.path.join(org_folder, folder))], key=os.path.getctime)
    return os.path.join(org_folder, model_folder)

def get_dockerfile_path(model_folder):
    for root, _, files in os.walk(model_folder):
        for file in files:
            if file.lower() == "dockerfile":
                return os.path.join(root)
    return None

if __name__ == "__main__":

    # Get the script's current file path
    script_path = os.path.abspath(__file__)

    # Navigate two folders back to the root of the repository
    root_path = os.path.dirname(os.path.dirname(script_path))

    # Move to the root directory
    os.chdir(root_path)

    # Move two directories back to the desired location
    os.chdir("..")
    
    model_folder = get_latest_model_name()

    dockerfile_path = get_dockerfile_path(model_folder)
    if dockerfile_path:
        print(dockerfile_path)
    else:
        print("Dockerfile not found.")