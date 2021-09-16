REGISTRY="\"\""
SSH_KEY="\"\""
SERVER_USER="\"\""
SERVER_IP="\"\""
DOCKER_DIR="\"\""
MOUNT_DIR="\"\""
JSON="\"\""

EksKube:
	@sh -c 'echo " mkdir -p ${OSSIEHOME}/{aws,.kube}"'
	@mkdir -p ${OSSIEHOME}/{aws,.kube}
	@echo " Creating empty placeholder files for k8s"
	# aws config
	@sh -c 'echo "[profile redhawk]" > ${OSSIEHOME}/aws/config.fake'
	@sh -c 'echo "region = <insert region your k8s cluster resides in here>" >> ${OSSIEHOME}/aws/config.fake'
	@sh -c 'echo "output = table" >> ${OSSIEHOME}/aws/config.fake'
	# aws credentials
	@sh -c 'echo "[redhawk]" > ${OSSIEHOME}/aws/credentials.fake'
	@sh -c 'echo "aws_access_key_id=YOURACCESSKEYIDHERE" >> ${OSSIEHOME}/aws/credentials.fake'
	@sh -c 'echo "aws_secret_access_key=YOURSECRETACCESSKEYHERE" >> ${OSSIEHOME}/aws/credentials.fake'
	@sh -c 'echo "Make sure you edit the config.fake and credentials.fake file, then remove the .fake suffixes when the files are ready" > ${OSSIEHOME}/aws/README'
	# kubectl config
	@sh -c 'echo "Configure your AWS credentials first" > ${OSSIEHOME}/.kube/README'
	@sh -c "echo \"Then run 'aws eks update-kubeconfig –name <EKS_Cluster_Name>'\" >> ${OSSIEHOME}/.kube/README"
	@sh -c 'echo "To create the kubectl config file. Move it to OSSIEHOME/.kube/config. This is where RH will look for the file." >> ${OSSIEHOME}/.kube/README'
	./build.py --cluster EksKube --registry ${REGISTRY}

#        sudo -E ./build.py --cluster EksKube --registry ${REGISTRY} --json ${JSON}


DockerSwarm:
	./build.py --cluster DockerSwarm --registry ${REGISTRY} --ssh_key ${SSH_KEY} --server_user ${SERVER_USER} --server_ip ${SERVER_IP}

Docker:
	./build.py --cluster Docker
