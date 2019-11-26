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
        //stage('Coding style') {
        //    steps {
        //        sh 'make clangcheck'
        //    }
        //}
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
