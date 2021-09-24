# EksKube
The EksKube plugin is designed to run REDHAWK waveforms on AWS Elastic Kubernetes Service (EKS) clusters.

# Building and Installing the Plugin

Application Factory ("make install" command may need sudo)
```bash
$ cd core-framework/redhawk/src/base/plugin/clustermgr/clustertype
$ ./build.py EksKube
$ ./reconf && ./configure && make && make install
```
Sandbox ("make" command may need sudo)
```bash
$ cd core-framework/redhawk/src/base/framework/python/ossie/utils/sandbox/clustertype
$ make FILE=EksKube
```

This will compile and install the Application Factory and Sandbox plugins for the user. The plugins are built in a specific location in core-framework (`core-framework/redhawk/src/base/plugin/clustermgr/`and `core-framework/redhawk/src/base/framework/python/ossie/utils/sandbox/`) and are both installed to `${OSSIEHOME}/lib`

# Plugin Specifics
## Dependencies
1. The [kubectl binary](https://kubernetes.io/docs/tasks/tools/install-kubectl-linux/) installed on your local REDHAWK system on your $PATH
2. The [aws cli binary](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2-linux.html) installed on your local REDHAWK system on your $PATH

## The cluster.cfg file
```bash
cd core-framework/redhawk/src/base/cfg
make EksKube REGISTRY="<your-registry>" JSON=""
```
This will properly set the top section to use the EksKube plugin and pass in the assortment of arguments to setup the cluster.cfg file.

NOTE that `make EksKube` will create a default cluster.cfg file full of blank values (empty strings "") and `make` with all the variables after it render a complete cluster.cfg file with those values set.

## cluster.cfg file variables
The top section of the file should specify that the EksKube plugin is desired like so:
```
[CLUSTER]
name = EksKube
```
| Variable         | Example Value | Description |
|------------------|----------------|-------|
| registry         | sample.amazonaws.com/my_context | Base URI for the container repository |
| tag              | latest | The image tag used (common across components) |
| dockerconfigjson |  | The auth field of your ~/.docker/config.json (may not be necessary) |


In AWS, the registry is found in AWS Elastic Container Services->Repositories->URI.
This is the image registry that EKS will use to find images to deploy as containers.

The tag "latest" is added by default by the image creation Dockerfiles included with RH.

The variable dockerconfigjson is not needed in most configurations.
If deployment fails due to a docker authorization issue, populate this element with the auth field from the docker configuration json, otherwise leave an empty string as its value.

## Environment Configuration

The Domain Manager and Python sandbox invoke EKS commands directly by using `aws` and `kubectl` commands.
The environment for the user account exercising either the Domain Manager process or the Python sandbox session must be configured to successfully interact with the desired EKS cluster.
This configuration includes the application of whatever authorization certificates are necessary.

The configuration should include the hidden directories `.aws` and `.kube` in the user account's home directory.
These directories contain information like user credentials and EKS cluster information.

<!-- Sourcing the `$OSSIHOME/etc/profile.d/redhawk.sh` file will set 4 environment variables Domain Manager will use to direct the control of the aws and kubectl binaries it invokes:
1. AWS_PROFILE=redhawk
2. AWS_CONFIG_FILE=/usr/local/redhawk/core/aws/config
3. AWS_SHARED_CREDENTIALS_FILE=/usr/local/redhawk/core/aws/credentials
4. KUBECONFIG=/usr/local/redhawk/core/.kube/config

If these variables are not set, or the files they point to are corrupted or misconfigured, Domain Manager logs will infrom you that its attempt to use aws or kubectl binaries are bailing; this is likely to be caused by these environment variables being missing, set incorrectly, or the files being configured incorrectly, so double-check to ensure these are correct. -->

<!-- ## $OSSIEHOME additions
The [plugin install process](#building-and-installing-the-plugin) will install new files into $OSSIEHOME. Aside from the cluster.cfg file, the EksKube plugin will install two new directories:
1. `$OSSIHOME/aws`: Contains AWS-specific files used by the `aws` CLI binary. Template files are generated at install-time and you will need to edit these files with your appropriate values.
2. `$OSSIHOME/.kube`: Contains the configuration file that instructs the `kubectl` binary how to interact with your EKS cluster.

#### aws Directory
You should see the following files in the `$OSSIEHHOME/aws` directory:
* config.fake
* credentials.fake
* README.md -->

<!-- The README simply explains to edit the *.fake suffix files to proper values and to drop the .fake suffix in order for the plugin to run properly. The EksKube plugin leverages the kubectl and aws cli binaries, meaning Domain Manager (which imports the plugin) uses these binaries. [See the official AWS documentation on configuring the aws cli binary.](https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-quickstart.html) The kubectl binary uses the aws binary for the EKS cluster according to the KUBECONFIG file (discussed later). The aws binary needs an Idenity and Access Management user setup that has the appropriate AWS permissions that kubectl expects to do its job. The plugin expects the `credentials` file, pointed to by the AWS_SHARED_CREDENTIALS_FILE environment variable, to contain [a named profile](https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-profiles.html) called `redhawk`. An example of your properly fixed `credentials` file (note the dropped .fake suffix) might look like:
```
[redhawk]
aws_access_key_id=<Your_redhawk_IAM_user's_key_id>
aws_secret_access_key=<Your_redhawk_IAM_user's_secret_key>
```
The `$OSSIHOME/aws/config` file contains additional configuration used for the aws cli binary. An example config file might look like:
```
[profile redhawk]
region = us-gov-west-1
output = table
```
The important notes for this file are:
* The configurations apply to the redhawk profile (IAM user)
* The region corresponds to [the AWS region](https://aws.amazon.com/about-aws/global-infrastructure/regions_az/) where your EKS cluster is running

#### .kube Directory
You should see the following files in the `$OSSIHOME/.kube` directory:
* config
README

The README explains how to use the [eksctl binary](https://docs.aws.amazon.com/eks/latest/userguide/eksctl.html) you presumably used to build your EKS cluster to generate a kube config file at this path for kubectl to use. The KUBECONFIG environment variable points to this config file. The eksctl binary will generate the config file at ~/.kube/config by default, so simply relocate that file to this path. -->

<!-- ## Credentials
The EksKube plugin needs the following credentials:
1. An AWS IAM user called `redhawk` with the appropriate permissions (see below)
2. An EKS cluster that has had its[aws-auth ConfigMap](https://docs.aws.amazon.com/eks/latest/userguide/add-user-role.html) updated to give the redhawk IAM user sufficient privileges in the k8s cluster
3. A valid dockerconfigjson auth string used to authenticate `docker pull` commands the k8s cluster executes to retrieve RH component images (if using images in a private registry)

### AWS IAM User
The redhawk AWS IAM user can use the following IAM policies as a starting point for its permissions needed:
1. The AWS-Managed Policy: AmazonEC2ContainerRegistryReadOnly (Allows the `redhawk` IAM user to `docker pull` from ECR)
2. A self-managed policy called `EksUser` that allows the redhawk user to query information about the EKS cluster
```
    "Version": "2012-10-17",
    "Statement": [
        {
            "Sid": "VisualEditor0",
            "Effect": "Allow",
            "Action": [
                "eks:DescribeNodegroup",
                "eks:ListNodegroups",
                "eks:ListUpdates",
                "eks:DescribeUpdate",
                "eks:DescribeCluster"
            ],
            "Resource": [
                "arn:aws-us-gov:eks:*:<AWS_Account_Number>:cluster/*",
                "arn:aws-us-gov:eks:*:<AWS_Account_Number>:nodegroup/*/*/*"
            ]
        },
        {
            "Sid": "VisualEditor1",
            "Effect": "Allow",
            "Action": "eks:ListClusters",
            "Resource": "*"
        }
    ]
}
```

### aws-auth ConfigMap
The `aws-auth` [ConfigMap](https://docs.aws.amazon.com/eks/latest/userguide/add-user-role.html) can add the redhawk IAM user to the `system:masters` group to give it complete access to the k8s cluster; not recommended for production clusters with sensitive and isolated workloads.
```
mapUsers:
----
- userarn: arn:aws-us-gov:iam::<AWS_ACCOUNT_ID>:user/redhawk
  username: redhawk
  groups:
    - system:masters
```

### dockerconfigjson variable
In a nutshell:
1. Update your system's `~/.docker/config.json` file
2. Install the cluster.cfg file and use the updated `~/.docker/config.json` to set its value

The `build.py` file at `core-framework/redhawk/src/base/cfg/build.py` can accept a `--json` argument to help update the dockerconfigjson variable for the Eks plugin: the dockerconfigjson variable. This variable is used in the Eks plugin to generate a Secret from your local system's `~/.docker/config.json` file, and that Secret is what authorizes k8s to perform a `docker pull` and retrieve your desired RH Component docker image from a private Docker Registry that requires authentication (if you are using images hosted on DockerHub that do not require authentication to pull, then this variable is not needed and the Secret yaml generated will be invalid and not used).

Your system's `~/.docker/config.json` file is updated whenever your run a `docker login <MyRegistry>` command. Depending on the Docker Registry used to house your RH Component docker images, you may use the standard `docker login` command syntax or something Registry-specific. For example, for images stored in AWS' [Elastic Container Registry (ECR)](https://aws.amazon.com/ecr/), the approach [for authenticating looks different](https://aws.amazon.com/blogs/compute/authenticating-amazon-ecr-repositories-for-docker-cli-with-credential-helper/). Regardless of how you update your system's `~/.docker/config.json` file, **you should do so prior to installing the Cluster Configuration File** so that it gets the proper auth value set in its dockerconfigjson variable.

Once you have your `~/.docker/config.json` updated, install the cluster.cfg file with these steps:
```
cd core-framework/redhawk/src/base/cfg/
sudo -E ./build.py --cluster EksKube --json `base64 -w0 ~/.docker/config.json`
```
This will extract the `auth` field from your `~/.docker/config.json` to pass it as an argument to the install file, which will install the Cluster Configuration File to `$OSSIHOME/cluster.cfg`. -->

<!-- ## Networking
EKS uses [AWS's VPC CNI](https://docs.aws.amazon.com/eks/latest/userguide/pod-networking.html) by default which provides a flat network. The caveat to this setup is that [EKS also enables SNAT by default](https://docs.aws.amazon.com/eks/latest/userguide/external-snat.html), which interferes with pods' ability to talk to an OmniORB running on your local system. This should be disabled by enabling "External SNAT" which stops k8s from snating pods' IPs that are attempting to communicate outside of the cluster.
```
kubectl set env daemonset -n kube-system aws-node AWS_VPC_K8S_CNI_EXTERNALSNAT=true
```

This configuration is ideal for running REDHAWK services external to the cluster (on your local REDHAWK system). -->

## Networking

Communications between Pods (individual component instances) in the EKS cluster is simple in most EKS configurations.
If the Domain Manager or Python sandbox session reside outside the EKS cluster, it may be necessary to change the host computer's firewall session to allow communications to initiate from the EKS cluster's Pod instances to the host computer.
