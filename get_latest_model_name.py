import os

def get_latest_model_name():
    org_folder = max([folder for folder in os.listdir('.') if os.path.isdir(folder) and folder != 'objects'], key=os.path.getctime)
    
    # Cd into the org folder
    os.chdir(org_folder)

    # List all files
    model_n = os.listdir('.')[0]

    if model_n == 'objects':
        exit()
    else:
        return model_n

if __name__ == "__main__":

    print(get_latest_model_name())