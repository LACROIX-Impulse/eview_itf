pipeline {
    agent {
        label 'sdk-ecube-0.1'
    }
    environment {
        SYSROOTS = "${env.SYSROOTS_0_1}"
    }
    triggers {
        bitbucketPush()
    }
    stages {
        stage('Build (Release)') {
            steps {
                sh '''
                    LD_LIBRARY_PATH= . ${SYSROOTS}/../environment-setup-aarch64-poky-linux
                    make
                '''
            }
        }
        stage('Coding style') {
            steps {
                sh 'make clangcheck'
            }
        }
        stage('Package Delivery') {
            steps {
                sh '''
                LD_LIBRARY_PATH= . ${SYSROOTS}/../environment-setup-aarch64-poky-linux
                make ipk
                '''
            }
        }
        stage('Publish') {
            when {
                anyOf {
                    branch "master"
                    branch "release/*"
                    branch "feature/EMIRROR-502-eviewitf-add-ipk-generation-and-publish-to-nexus"
                }
            }
            environment {
               VERSION = sh(script: 'make version', , returnStdout: true).trim()
            }
            steps {
                nexusPublisher nexusInstanceId: 'Nexus3', nexusRepositoryId: 'eCube-releases', packages:
                    [[$class: 'MavenPackage', mavenAssetList: [[classifier: '', extension: 'ipk', filePath: "build/eviewitf-${env.VERSION}.ipk"]],
                        mavenCoordinate: [artifactId: 'eviewitf', groupId: 'com.esoftthings.ecube', packaging: 'ipk', version: "${env.VERSION}"]]]
            }
        }
    }
    post {
        always {
            script {
                currentBuild.result = currentBuild.result ?: 'SUCCESS'
                deleteDir();
            }
        }
    }
}
