REGISTRY="\"\""
SSH_KEY="\"\""
SERVER_USER="\"\""
SERVER_IP="\"\""
DOCKER_DIR="\"\""
MOUNT_DIR="\"\""
JSON="\"\""

EksKube:
	@sudo sh -c 'echo " mkdir -p /usr/local/redhawk/core/{aws,.kube}"'
	@sudo mkdir -p /usr/local/redhawk/core/{aws,.kube}
	@echo " Creating empty placeholder files for k8s"
	# aws config
	@sudo sh -c 'echo "[profile redhawk]" > /usr/local/redhawk/core/aws/config.fake'
	@sudo sh -c 'echo "region = <insert region your k8s cluster resides in here>" >> /usr/local/redhawk/core/aws/config.fake'
	@sudo sh -c 'echo "output = table" >> /usr/local/redhawk/core/aws/config.fake'
	# aws credentials
	@sudo sh -c 'echo "[redhawk]" > /usr/local/redhawk/core/aws/credentials.fake'
	@sudo sh -c 'echo "aws_access_key_id=YOURACCESSKEYIDHERE" >> /usr/local/redhawk/core/aws/credentials.fake'
	@sudo sh -c 'echo "aws_secret_access_key=YOURSECRETACCESSKEYHERE" >> /usr/local/redhawk/core/aws/credentials.fake'
	@sudo sh -c 'echo "Make sure you edit the config.fake and credentials.fake file, then remove the .fake suffixes when the files are ready" > /usr/local/redhawk/core/aws/README'
	# kubectl config
	@sudo sh -c 'echo "Configure your AWS credentials first" > /usr/local/redhawk/core/.kube/README'
	@sudo sh -c "echo \"Then run 'aws eks update-kubeconfig –name <EKS_Cluster_Name>'\" >> /usr/local/redhawk/core/.kube/README"
	@sudo sh -c 'echo "To create the kubectl config file. Move it to OSSIEHOME/.kube/config. This is where RH will look for the file." >> /usr/local/redhawk/core/.kube/README'
	sudo -E ./build.py --cluster EksKube --registry ${REGISTRY} --json ${JSON}

DockerSwarm:
	sudo -E ./build.py --cluster DockerSwarm --registry ${REGISTRY} --ssh_key ${SSH_KEY} --server_user ${SERVER_USER} --server_ip ${SERVER_IP}

Docker:
	sudo -E ./build.py --cluster Docker --docker_dir ${DOCKER_DIR} --mount_dir ${MOUNT_DIR}
