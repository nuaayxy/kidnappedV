/**
 * particle_filter.cpp
 *
 * Created on: Dec 12, 2016
 * Author: Tiffany Huang
 */

#include "particle_filter.h"

#include <math.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "helper_functions.h"

using std::string;
using std::vector;
using std::normal_distribution;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
  /**
   * TODO: Set the number of particles. Initialize all particles to 
   *   first position (based on estimates of x, y, theta and their uncertainties
   *   from GPS) and all weights to 1. 
   * TODO: Add random Gaussian noise to each particle.
   * NOTE: Consult particle_filter.h for more information about this method 
   *   (and others in this file).
   */
  num_particles = 100;  // TODO: Set the number of particles
  std::default_random_engine gen;

// This line creates a normal (Gaussian) distribution for x

    std::normal_distribution<double> dist_x(x, std[0]);
    std::normal_distribution<double> dist_y(y, std[1]);
    std::normal_distribution<double> dist_theta(theta, std[2]);


    for (int i = 0; i < num_particles; ++i)
    {
         double sample_x, sample_y, sample_theta;
          sample_x = dist_x(gen);
          sample_y = dist_y(gen);
          sample_theta = dist_theta(gen);

          Particle sample = {i, sample_x,sample_y, sample_theta, 1.0};
          particles.push_back(sample);
          // Print your samples to the terminal.
          std::cout << "Sample " << i + 1 << " " << sample_x << " " << sample_y << " "
                    << sample_theta << std::endl;
}

}

void ParticleFilter::prediction(double delta_t, double std_pos[], 
                                double velocity, double yaw_rate) {
  /**
   * TODO: Add measurements to each particle and add random Gaussian noise.
   * NOTE: When adding noise you may find std::normal_distribution 
   *   and std::default_random_engine useful.
   *  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
   *  http://www.cplusplus.com/reference/random/default_random_engine/
   */
    std::default_random_engine gen;
    normal_distribution<double> dist_x(0, std_pos[0]);
    normal_distribution<double> dist_y(0, std_pos[1]);
    normal_distribution<double> dist_theta(0, std_pos[2]);



    for(int i = 0;i<num_particles;i++)
    {
        particles[i].x = dist_x(gen) + particles[i].x + velocity/yaw_rate*(sin(particles[i].theta + yaw_rate*delta_t) - sin(particles[i].theta));
        particles[i].y = dist_y(gen) + particles[i].y + velocity/yaw_rate*(cos(particles[i].theta) - cos(particles[i].theta + yaw_rate*delta_t));
        particles[i].theta = dist_theta(gen) + particles[i].theta + yaw_rate*delta_t;
    }



   

}

void ParticleFilter::dataAssociation(vector<LandmarkObs> predicted,
                                     vector<LandmarkObs>& observations) {
  /**
   * TODO: Find the predicted measurement that is closest to each 
   *   observed measurement and assign the observed measurement to this 
   *   particular landmark.
   * NOTE: this method will NOT be called by the grading code. But you will 
   *   probably find it useful to implement this method and use it as a helper 
   *   during the updateWeights phase.
   */
//    for(int i = 0; i<predicted.size() ;i ++)
//    {
//        double min_dist = 9999999;
//        int min_idx = -1;
//        for(int j = 0; j<observations.size();j++)
//        {

//            double distance = dist(predicted[i].x, predicted[i].y, observations[j].x, observations[j].y );
//            if(distance < min_dist)
//            {

//                min_idx = observations[j].id;
//                min_dist = distance;
//            }
//        }

//        predicted[i].id = min_idx;
//    }


}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
                                   const vector<LandmarkObs> &observations, 
                                   const Map &map_landmarks) {
  /**
   * TODO: Update the weights of each particle using a mult-variate Gaussian 
   *   distribution. You can read more about this distribution here: 
   *   https://en.wikipedia.org/wiki/Multivariate_normal_distribution
   * NOTE: The observations are given in the VEHICLE'S coordinate system. 
   *   Your particles are located according to the MAP'S coordinate system. 
   *   You will need to transform between the two systems. Keep in mind that
   *   this transformation requires both rotation AND translation (but no scaling).
   *   The following is a good resource for the theory:
   *   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
   *   and the following is a good resource for the actual equation to implement
   *   (look at equation 3.33) http://planning.cs.uiuc.edu/node99.html
   */

    // define coordinates and theta
    for(int i =0; i<num_particles;i++)
    {
        double x_part, y_part, x_obs, y_obs, theta;
        x_part = particles[i].x;
        y_part = particles[i].y;
        theta =  particles[i].theta;
        particles[i].weight = 1;

        for(int j = 0; j<observations.size();j++)
        {
            x_obs = observations[j].x;
            y_obs = observations[j].y;

            // transform to map x coordinate
            double x_map_particle = x_part + (cos(theta) * x_obs) - (sin(theta) * y_obs);

            // transform to map y coordinate
            double y_map_particle = y_part + (sin(theta) * x_obs) + (cos(theta) * y_obs);

            //use nearest neibgough method to match with map observations
            particles[i].sense_x.push_back(x_map_particle);
            particles[i].sense_y.push_back(y_map_particle);

            int map_size = map_landmarks.landmark_list.size();;
//for each observation find the closet map_landmarks
            double min_dist = 99999;
            int min_idx =  -1;
            double min_x = 0;
            double min_y = 0;
            for(int k = 0;k< map_size; k++)
            {
                double map_x = map_landmarks.landmark_list[k].x_f;
                double map_y = map_landmarks.landmark_list[k].y_f;
                int map_id = map_landmarks.landmark_list[k].id_i;

                std::cout<<k<<"    "<<map_id<<std::endl;

                double distance = dist(map_x, map_y, x_map_particle, y_map_particle );
                if(distance < min_dist)
                {

                    min_idx = map_id;
                    min_dist = distance;
                    min_x = map_x;
                    min_y = map_y;
                }

            }

            //push back the associated map landmark id relative to the observation
            particles[i].associations.push_back(min_idx);

//            double min_map_x =
            particles[i].weight *= multiv_prob(std_landmark[0], std_landmark[1], x_map_particle, y_map_particle, min_x, min_y);





            //find the map landmark id
//            particles[i].associations.push_back();


            //then calculate the multivariate gaussian probabity


        }

        weights.push_back(particles[i].weight);



    }

        //normalize the weights of each particle so they sum to 1
        double sum_of_elems = std::accumulate(weights.begin(), weights.end(), 0.0);
        for(int i = 0;i<weights.size();i++)
        {
            weights[i]/=sum_of_elems;
        }

}

void ParticleFilter::resample() {
  /**
   * TODO: Resample particles with replacement with probability proportional 
   *   to their weight. 
   * NOTE: You may find std::discrete_distribution helpful here.
   *   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
   */

//p3 = []
//max_w = max(w)
//for i in range(N):
//    beta = random.uniform(0,max_w )
//    index = 0
//    while w[index] < beta:
//        beta = beta - w[index]
//        index = index + 1
//        index %= N
//    p3.append(p[index])

// Create the distribution with those weights
    std::default_random_engine gen;
//    std::random_device rd;
//    std::mt19937 gen(rd());
    double max_weight = *max_element(std::begin(weights), std::end(weights));
//    std::discrete_distribution<> distribution_weight(0, max_weight*2);
    std::uniform_real_distribution<double> distribution(0.0,max_weight*2);

    vector<Particle> particles_copy(particles);
    particles.clear();
    for(int i = 0;i<num_particles;i++)
    {
        double number = distribution(gen);
        int idx = 0;

        while(weights[idx]<number)
        {
            number = number = weights[idx];
            idx++;
            idx%=num_particles;
        }
        particles.push_back(particles_copy[idx]);

    }





}

void ParticleFilter::SetAssociations(Particle& particle, 
                                     const vector<int>& associations, 
                                     const vector<double>& sense_x, 
                                     const vector<double>& sense_y) {
  // particle: the particle to which assign each listed association, 
  //   and association's (x,y) world coordinates mapping
  // associations: The landmark id that goes along with each listed association
  // sense_x: the associations x mapping already converted to world coordinates
  // sense_y: the associations y mapping already converted to world coordinates
  particle.associations= associations;
  particle.sense_x = sense_x;
  particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best) {
  vector<int> v = best.associations;
  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<int>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}

string ParticleFilter::getSenseCoord(Particle best, string coord) {
  vector<double> v;

  if (coord == "X") {
    v = best.sense_x;
  } else {
    v = best.sense_y;
  }

  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<float>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}
