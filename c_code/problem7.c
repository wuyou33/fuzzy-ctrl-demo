/* 
 * CI Class WSS2013-14
 * 
 * Fuzzy logic control homework
 * 
 * Mobile robot trajectory tracking
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

/* handy macros */
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define MAX_TRAJ_SIZE	1000
#define NUM_RULES	4

/* robot constants */
#define ROBOT_WHEEL_RADIUS 	0.04 /* m */
#define	ROBOT_BASE_SIZE		0.16 /* m */

	

/* helper types */

/* specify the type of aggregator */
enum 
{
	min = 1,
	prod
};

/* membership wedges */
enum
{
	smalld = 11,
	larged = 12,
	smallt = 21,
	larget = 22
};

typedef struct 
{
	double crisp_val;	/* crisp value */
	int membership_func;	/* type of membership function */
}fuzzy_in;

typedef struct
{
	double x_pos;
	double y_pos;
	double theta;
}trajectory;

/* enum specifying the type of membership function */
enum
{
	test = 0,
	triangle = 1,	/* f(x,a,b,c) = max( min( x-a/b-a  , c-x/c-b ), 0) */
	trapezoid,	/* f(x,a,b,c,d) = max( min( x-a/b-a, min( 1, d-x/d-c )), 0) */
	gauss		/* f(x, sigma, mean) = exp( -(x-mean)^2 / 2*sigma^2) */
};

/* compute translation error */
double compute_de(trajectory ref, trajectory cur)
{
	return (double)sqrt((pow(ref.x_pos - cur.x_pos, 2) + pow(ref.y_pos - cur.y_pos, 2)));
}

/* compute the heading error */
double compute_thetae(trajectory ref, trajectory cur)
{
	return (double)(ref.theta - cur.theta);
}

/* aggregate the antecedents and compute the weight (membership degree) */
double compute_aggregation(int type, double memb_val_in1, double memb_val_in2)
{
	return (type==min)?min(memb_val_in1, memb_val_in2):(memb_val_in1*memb_val_in2);
}

/* compute the membership vf for a crisp input */
double compute_membership(int type, fuzzy_in* in)
{
	double out = 0.0f;
	/* cherck the type of the membership */
	switch(type){
		case 0:
			switch(in->membership_func){
				case 11:
					out = min(max(0, -(in->crisp_val-80)/160), 1);
				break;
				case 12:
					out = 1 - min(max(0, -(in->crisp_val-80)/160), 1);
				break;
				case 21:
					out = (-abs(in->crisp_val)+180)/180;
				break;
				case 22:
					out = 1 -  (-abs(in->crisp_val)+180)/180;
				break;
			}
		break;
		case 1:
	
		break;

		case 2:


		break;


		case 3:

		break;
	}
	return out; 
}

/* entry point */
int main(int argc, char** argv)
{
	
	/* create the reference trajectory */
	trajectory *ref_trajectory = (trajectory*)calloc(MAX_TRAJ_SIZE, sizeof(trajectory));
	/* create the robot trajectory */
	trajectory *robot_trajectory = (trajectory*)calloc(MAX_TRAJ_SIZE, sizeof(trajectory));	

	/* input data samples counter */
	int input_idx = 0;	

	/* fuzzy controller vars */
	double w1 = 0.0f, w2 = 0.0f, w3 = 0.0f, w4 = 0.0f;
	/* control signals for each rule */
	double *u1 = (double*)calloc(2,sizeof(double));
	double *u2 = (double*)calloc(2,sizeof(double));
	double *u3 = (double*)calloc(2,sizeof(double));
	double *u4 = (double*)calloc(2,sizeof(double));
	double *u_fin = (double*)calloc(2, sizeof(double));

	/* the 2 fuzzified inputs, de and thetae */
	fuzzy_in *de = (fuzzy_in*)calloc(1, sizeof(fuzzy_in));
	fuzzy_in *thetae = (fuzzy_in*)calloc(1, sizeof(fuzzy_in));
	
	/* robot params */
	double robot_lin_vel = 0.0f;
	double robot_rot_vel = 0.0f;	
	double Kde[NUM_RULES] = {0.25,0.25,0.50,0.5};
	double Kte[NUM_RULES] = {0.12,0.25,0.12,0.25};


	/* get input data from stdin */
	while(scanf("%lf,%lf,%lf\n", &(ref_trajectory[input_idx].x_pos), 
				     &(ref_trajectory[input_idx].y_pos),
				     &(ref_trajectory[input_idx].theta))>0){
		/* check if max size was reached */
		if(++input_idx == MAX_TRAJ_SIZE) break;
	}
	
	/* init the robo trajectory with the first reference trajectory point */
	robot_trajectory[0].x_pos = ref_trajectory[0].x_pos;
	robot_trajectory[0].y_pos = ref_trajectory[0].y_pos;
	robot_trajectory[0].theta = ref_trajectory[0].theta;

	/* main control loop simulation */
	for(int t = 1; t<input_idx; ++t){
		/* the 2 inputs in the fuzzy controller are the 2 error values */
		de->crisp_val = compute_de(ref_trajectory[t-1], robot_trajectory[t-1]);
		thetae->crisp_val = compute_thetae(ref_trajectory[t-1], robot_trajectory[t-1]);
		
		/* fire the rules */
		
		/* first rule */
		de->membership_func = smalld;
		thetae->membership_func = smallt;
	
		w1 = compute_aggregation(min,
					 compute_membership(test,de),
					 compute_membership(test,thetae)
					);
		u1[1] = Kde[1]*(de->crisp_val) + Kte[1]*(thetae->crisp_val);
		u1[2] = Kde[1]*(de->crisp_val) - Kte[1]*(thetae->crisp_val);


		/* second rule */
		de->membership_func = smalld;
		thetae->membership_func = larget;
	
		w2 = compute_aggregation(min,
					 compute_membership(test,de),
					 compute_membership(test,thetae)
					);
		u2[1] = Kde[2]*de->crisp_val + Kte[2]*thetae->crisp_val;
		u2[2] = Kde[2]*de->crisp_val - Kte[2]*thetae->crisp_val;


		/* third rule */
		de->membership_func = larged;
		thetae->membership_func = smallt;
	
		w3 = compute_aggregation(min,
					 compute_membership(test,de),
					 compute_membership(test,thetae)
					);
		u3[1] = Kde[3]*de->crisp_val + Kte[3]*thetae->crisp_val;
		u3[2] = Kde[3]*de->crisp_val - Kte[3]*thetae->crisp_val;


		/* fourth rule */
		de->membership_func = larged;
		thetae->membership_func = larget;
	
		w4 = compute_aggregation(min,
					 compute_membership(test,de),
					 compute_membership(test,thetae)
					);
		u4[1] = Kde[4]*de->crisp_val + Kte[4]*thetae->crisp_val;
		u4[2] = Kde[4]*de->crisp_val - Kte[4]*thetae->crisp_val;

		/* compute output */
	        u_fin[1] = (w1*u1[1] + w2*u2[1] + w3*u3[1] + w4*u4[1])/(w1+w2+w3+w4);
		u_fin[2] = (w1*u1[2] + w2*u2[2] + w3*u3[2] + w4*u4[2])/(w1+w2+w3+w4);
			
		/* send command to the robot (kinematic control) */
		robot_lin_vel = (u_fin[1] + u_fin[2])/2;
		robot_rot_vel = (u_fin[1] - u_fin[2])/ROBOT_BASE_SIZE;
		robot_trajectory[t].x_pos = robot_trajectory[t-1].x_pos +
					    robot_lin_vel*cos(robot_trajectory[t-1].theta);
		robot_trajectory[t].y_pos = robot_trajectory[t-1].y_pos + 
					    robot_lin_vel*sin(robot_trajectory[t-1].theta);
		robot_trajectory[t].theta = robot_trajectory[t-1].theta + 
					    robot_rot_vel;
	
	}

	FILE *fin = fopen("input.log","w");
	for(int i = 0;i<input_idx;i++){
		fprintf(fin, "%lf,%lf,%lf\n", ref_trajectory[i].x_pos, ref_trajectory[i].y_pos, ref_trajectory[i].theta);
	}
	fclose(fin);

	FILE *fout = fopen("output.log","w");
	for(int i = 0;i<input_idx;i++){
		fprintf(fin, "%lf,%lf,%lf\n", robot_trajectory[i].x_pos, robot_trajectory[i].y_pos, robot_trajectory[i].theta);
	}
	fclose(fout);


	return (EXIT_SUCCESS);
}