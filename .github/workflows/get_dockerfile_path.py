import os
import yaml

def extract_organization_name(pull_request_description):
    try:
        data = yaml.safe_load(pull_request_description)
        model_details = data.get('Model Details', {})
        org_name = model_details.get('Organization Name', None)
        return org_name
    except Exception as e:
        print(f"Error while parsing YAML: {e}")
        return None

def get_latest_model_name():
    # Get the pull request description from the github action environment variables
    pull_request_description = os.environ.get("pr_body", '')


    org_folder = extract_organization_name(pull_request_description)
    
    # Cd into the org folder
    os.chdir(org_folder)

    # Assign current directory to model_n
    model_n = os.getcwd()

    if model_n == 'objects':
        exit()
    else:
        return model_n
def get_dockerfile_path(model_folder):
    for root, _, files in os.walk(model_folder):
        for file in files:
            if file.lower() == "dockerfile":
                return os.path.join(root)
    return None


if __name__ == "__main__":
    model_folder = get_latest_model_name()

    dockerfile_path = get_dockerfile_path(model_folder)
    if dockerfile_path:
        print(dockerfile_path)
    else:
        print("Dockerfile not found.")