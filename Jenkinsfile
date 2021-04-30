pipeline {
    agent {
        label 'sdk-ecube-0.17'
    }
    environment {
        SYSROOTS = "${env.SYSROOTS_0_17}"
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
        stage('Documentation') {
            steps {
                sh 'make doc'
            }
        }
        stage('Package Delivery') {
            steps {
                sh '''
                LD_LIBRARY_PATH= . ${SYSROOTS}/../environment-setup-aarch64-poky-linux
                make package
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
                    [[$class: 'MavenPackage', mavenAssetList: [[classifier: '', extension: 'tar', filePath: "build/eviewitf-${env.VERSION}.tar"]],
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
