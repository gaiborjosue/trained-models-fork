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
    pull_request_description = os.environ.get("pr_body", '')

    org_folder = extract_organization_name(pull_request_description)
    
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