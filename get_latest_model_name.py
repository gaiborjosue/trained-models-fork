import os

def get_latest_model_name():
    org_folder = max([folder for folder in os.listdir('.') if os.path.isdir(folder) and folder != 'objects'], key=os.path.getctime)
    model_folder = max([folder for folder in os.listdir(org_folder) if os.path.isdir(os.path.join(org_folder, folder))], key=os.path.getctime)
    return os.path.basename(model_folder)

if __name__ == "__main__":

    print(get_latest_model_name())