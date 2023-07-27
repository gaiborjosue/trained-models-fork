from Module.utils import get_latest_model_name, get_dockerfile_path

if __name__ == "__main__":
    model_folder = get_latest_model_name()

    dockerfile_path = get_dockerfile_path(model_folder)
    if dockerfile_path:
        print(dockerfile_path)
    else:
        print("Dockerfile not found.")