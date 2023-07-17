import os

def get_latest_model_name():
    # Get the script's current file path
    script_path = os.path.abspath(__file__)

    # Navigate two folders back to the root of the repository
    root_path = os.path.dirname(os.path.dirname(script_path))

    # Move to the root directory
    os.chdir(root_path)

    # Move two directories back to the desired location
    os.chdir("..")

    org_folder = max([folder for folder in os.listdir('.') if os.path.isdir(folder)], key=os.path.getctime)
    model_folder = max([folder for folder in os.listdir(org_folder) if os.path.isdir(os.path.join(org_folder, folder))], key=os.path.getctime)
    return os.path.basename(model_folder)

if __name__ == "__main__":

    print(get_latest_model_name())