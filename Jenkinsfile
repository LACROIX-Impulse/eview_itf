pipeline {
    agent {
        label 'sdk-ecube-0.12'
    }
    environment {
        SYSROOTS = "${env.SYSROOTS_0_12}"
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
                }
            }
            environment {
               VERSION = sh(script: 'make version', , returnStdout: true).trim()
            }
            steps {
                nexusPublisher nexusInstanceId: 'Nexus3', nexusRepositoryId: 'eCube-releases', packages:
                    [[$class: 'MavenPackage', mavenAssetList: [[classifier: '', extension: '.ipk.tar', filePath: "build/eviewitf-${env.VERSION}.ipk"]],
                        mavenCoordinate: [artifactId: 'eviewitf', groupId: 'com.esoftthings.ecube', packaging: 'tar', version: "${env.VERSION}"]]]
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
